#include "silo/query_engine/filter/expressions/mutation_profile.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/nof.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/horizontal_coverage_index.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
MutationProfile<SymbolType>::MutationProfile(
   std::optional<std::string> sequence_name,
   uint32_t distance,
   ProfileInput input
)
    : sequence_name(std::move(sequence_name)),
      distance(distance),
      input(std::move(input)) {}

template <typename SymbolType>
std::string MutationProfile<SymbolType>::toString() const {
   const std::string seq_prefix = sequence_name ? sequence_name.value() + ":" : "";
   const std::string input_str = std::visit(
      [](const auto& inp) -> std::string {
         using T = std::decay_t<decltype(inp)>;
         if constexpr (std::is_same_v<T, QuerySequenceInput>) {
            return "querySequence=" + inp.sequence.substr(0, 20) + "...";
         } else if constexpr (std::is_same_v<T, SequenceIdInput>) {
            return "sequenceId=" + inp.id;
         } else {
            return "mutations(count=" + std::to_string(inp.mutations.size()) + ")";
         }
      },
      input
   );
   return fmt::format("MutationProfile({}distance={},{})", seq_prefix, distance, input_str);
}

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> MutationProfile<SymbolType>::buildProfileFromQuerySequence(
   const storage::column::SequenceColumnPartition<SymbolType>& seq_partition
) const {
   const auto& query_sequence = std::get<QuerySequenceInput>(input).sequence;
   const size_t ref_len = seq_partition.metadata->reference_sequence.size();
   CHECK_SILO_QUERY(
      query_sequence.size() == ref_len,
      "querySequence length {} does not match the reference sequence length {} for {} "
      "MutationProfile",
      query_sequence.size(),
      ref_len,
      SymbolType::SYMBOL_NAME
   );

   std::vector<typename SymbolType::Symbol> profile;
   profile.reserve(ref_len);
   for (char character : query_sequence) {
      const auto symbol = SymbolType::charToSymbol(character);
      CHECK_SILO_QUERY(
         symbol.has_value(),
         "Invalid {} symbol '{}' in querySequence for MutationProfile",
         SymbolType::SYMBOL_NAME,
         character
      );
      profile.push_back(symbol.value());
   }
   return profile;
}

namespace {

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> reconstructSequenceAtRow(
   const storage::column::SequenceColumnPartition<SymbolType>& seq_col,
   uint32_t row_id
) {
   roaring::Roaring single_row;
   single_row.add(row_id);

   std::vector<std::string> sequences = {seq_col.local_reference_sequence_string};
   seq_col.vertical_sequence_index.overwriteSymbolsInSequences(sequences, single_row);
   seq_col.horizontal_coverage_index.template overwriteCoverageInSequence<SymbolType>(
      sequences, single_row
   );

   std::vector<typename SymbolType::Symbol> profile;
   profile.reserve(sequences[0].size());
   for (const char character : sequences[0]) {
      const auto sym = SymbolType::charToSymbol(character);
      SILO_ASSERT(sym.has_value());
      profile.push_back(sym.value());
   }
   return profile;
}

}  // namespace

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> MutationProfile<SymbolType>::buildProfileFromSequenceId(
   const storage::Table& table,
   const std::string& valid_sequence_name
) const {
   const auto& seq_id = std::get<SequenceIdInput>(input).id;
   const auto& pk_name = table.schema.primary_key.name;
   const auto pk_type = table.schema.primary_key.type;

   for (size_t partition_idx = 0; partition_idx < table.getNumberOfPartitions(); ++partition_idx) {
      const auto& partition = *table.getPartition(partition_idx);
      const auto& seq_col =
         partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

      std::optional<uint32_t> found_row_id;

      if (pk_type == schema::ColumnType::STRING) {
         const auto& pk_col = partition.columns.string_columns.at(pk_name);
         for (uint32_t row_id = 0; row_id < static_cast<uint32_t>(pk_col.numValues()); ++row_id) {
            if (pk_col.getValueString(row_id) == seq_id) {
               found_row_id = row_id;
               break;
            }
         }
      } else if (pk_type == schema::ColumnType::INDEXED_STRING) {
         const auto& pk_col = partition.columns.indexed_string_columns.at(pk_name);
         const auto bitmap_opt = pk_col.filter(std::optional<std::string>(seq_id));
         if (bitmap_opt.has_value() && !bitmap_opt.value()->isEmpty()) {
            found_row_id = bitmap_opt.value()->minimum();
         }
      } else {
         throw IllegalQueryException(fmt::format(
            "Unsupported primary key column type for {} MutationProfile sequenceId lookup",
            SymbolType::SYMBOL_NAME
         ));
      }

      if (found_row_id.has_value()) {
         return reconstructSequenceAtRow<SymbolType>(seq_col, found_row_id.value());
      }
   }

   CHECK_SILO_QUERY(
      false,
      "No sequence found with primary key '{}' in {} MutationProfile",
      seq_id,
      SymbolType::SYMBOL_NAME
   );
   SILO_UNREACHABLE();
}

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> MutationProfile<SymbolType>::buildProfileFromMutations(
   const storage::column::SequenceColumnPartition<SymbolType>& seq_partition
) const {
   const auto& mutation_list = std::get<MutationsInput>(input).mutations;
   const size_t ref_len = seq_partition.metadata->reference_sequence.size();

   // Start with a copy of the reference sequence
   std::vector<typename SymbolType::Symbol> profile(
      seq_partition.metadata->reference_sequence.begin(),
      seq_partition.metadata->reference_sequence.end()
   );

   for (const auto& mutation : mutation_list) {
      CHECK_SILO_QUERY(
         mutation.position_idx < ref_len,
         "{} MutationProfile mutation position {} is out of bounds (reference length {})",
         SymbolType::SYMBOL_NAME,
         mutation.position_idx + 1,
         ref_len
      );
      profile[mutation.position_idx] = mutation.symbol;
   }
   return profile;
}

template <typename SymbolType>
std::unique_ptr<Expression> MutationProfile<SymbolType>::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || table.schema.getDefaultSequenceName<SymbolType>(),
      "Database does not have a default sequence name for {} sequences. "
      "You need to provide the sequence name with the {} MutationProfile filter.",
      SymbolType::SYMBOL_NAME,
      SymbolType::SYMBOL_NAME
   );

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, table.schema);

   const auto& seq_partition =
      table_partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   // Build the profile sequence
   std::vector<typename SymbolType::Symbol> profile;
   if (std::holds_alternative<QuerySequenceInput>(input)) {
      profile = buildProfileFromQuerySequence(seq_partition);
   } else if (std::holds_alternative<SequenceIdInput>(input)) {
      profile = buildProfileFromSequenceId(table, valid_sequence_name);
   } else {
      profile = buildProfileFromMutations(seq_partition);
   }

   // For each position, build a "difference" child expression:
   // difference at pos = SymbolInSet(seq_name, pos, symbols NOT compatible with profile[pos])
   // where "compatible" means: symbols in AMBIGUITY_SYMBOLS[profile[pos]]
   ExpressionVector difference_children;
   for (size_t pos = 0; pos < profile.size(); ++pos) {
      const auto profile_symbol = profile[pos];

      // Skip positions where the profile has the missing/unknown symbol
      if (profile_symbol == SymbolType::SYMBOL_MISSING) {
         continue;
      }

      // Compute symbols that are NOT compatible with profile_symbol
      // (i.e., symbols that count as "definitely different" from profile_symbol)
      const auto& compatible_symbols = SymbolType::AMBIGUITY_SYMBOLS.at(profile_symbol);
      std::vector<typename SymbolType::Symbol> difference_symbols;
      for (const auto sym : SymbolType::SYMBOLS) {
         if (std::find(compatible_symbols.begin(), compatible_symbols.end(), sym) ==
             compatible_symbols.end()) {
            difference_symbols.push_back(sym);
         }
      }

      if (difference_symbols.empty()) {
         continue;
      }

      difference_children.push_back(std::make_unique<SymbolInSet<SymbolType>>(
         valid_sequence_name, static_cast<uint32_t>(pos), std::move(difference_symbols)
      ));
   }

   // Return Not(NOf(difference_children, distance+1, false))
   // = "at most 'distance' differences" (conservative)
   auto at_least_distance_plus_one = std::make_unique<NOf>(
      std::move(difference_children),
      static_cast<int>(distance) + 1,
      /*match_exactly=*/false
   );
   return std::make_unique<Negation>(std::move(at_least_distance_plus_one));
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> MutationProfile<SymbolType>::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/
) const {
   throw QueryCompilationException{
      "{} MutationProfile expression must be eliminated in the query rewrite phase",
      SymbolType::SYMBOL_NAME
   };
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
void from_json(const nlohmann::json& json, std::unique_ptr<MutationProfile<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.contains("distance"),
      "The field 'distance' is required in a {} MutationProfile expression",
      SymbolType::SYMBOL_NAME
   );
   CHECK_SILO_QUERY(
      json["distance"].is_number_unsigned(),
      "The field 'distance' in a {} MutationProfile expression must be an unsigned integer",
      SymbolType::SYMBOL_NAME
   );
   const uint32_t distance = json["distance"].get<uint32_t>();

   std::optional<std::string> seq_name;
   if (json.contains("sequenceName")) {
      seq_name = json["sequenceName"].get<std::string>();
   }

   const bool has_query_sequence = json.contains("querySequence");
   const bool has_sequence_id = json.contains("sequenceId");
   const bool has_mutations = json.contains("mutations");

   const int input_count = static_cast<int>(has_query_sequence) +
                           static_cast<int>(has_sequence_id) + static_cast<int>(has_mutations);

   CHECK_SILO_QUERY(
      input_count == 1,
      "Exactly one of 'querySequence', 'sequenceId', or 'mutations' must be provided in a {} "
      "MutationProfile expression, but {} were provided",
      SymbolType::SYMBOL_NAME,
      input_count
   );

   if (has_query_sequence) {
      CHECK_SILO_QUERY(
         json["querySequence"].is_string(),
         "The field 'querySequence' in a {} MutationProfile expression must be a string",
         SymbolType::SYMBOL_NAME
      );
      filter = std::make_unique<MutationProfile<SymbolType>>(
         seq_name,
         distance,
         typename MutationProfile<SymbolType>::QuerySequenceInput{
            json["querySequence"].get<std::string>()
         }
      );
      return;
   }

   if (has_sequence_id) {
      CHECK_SILO_QUERY(
         json["sequenceId"].is_string(),
         "The field 'sequenceId' in a {} MutationProfile expression must be a string",
         SymbolType::SYMBOL_NAME
      );
      filter = std::make_unique<MutationProfile<SymbolType>>(
         seq_name,
         distance,
         typename MutationProfile<SymbolType>::SequenceIdInput{json["sequenceId"].get<std::string>()
         }
      );
      return;
   }

   // has_mutations == true
   CHECK_SILO_QUERY(
      json["mutations"].is_array(),
      "The field 'mutations' in a {} MutationProfile expression must be an array",
      SymbolType::SYMBOL_NAME
   );

   std::vector<typename MutationProfile<SymbolType>::Mutation> mutations;
   for (const auto& mut_json : json["mutations"]) {
      CHECK_SILO_QUERY(
         mut_json.contains("position"),
         "Each mutation in a {} MutationProfile expression must have a 'position' field",
         SymbolType::SYMBOL_NAME
      );
      CHECK_SILO_QUERY(
         mut_json["position"].is_number_unsigned(),
         "The field 'position' in a {} MutationProfile mutation must be an unsigned integer",
         SymbolType::SYMBOL_NAME
      );
      CHECK_SILO_QUERY(
         mut_json.contains("symbol"),
         "Each mutation in a {} MutationProfile expression must have a 'symbol' field",
         SymbolType::SYMBOL_NAME
      );
      CHECK_SILO_QUERY(
         mut_json["symbol"].is_string(),
         "The field 'symbol' in a {} MutationProfile mutation must be a string",
         SymbolType::SYMBOL_NAME
      );

      const uint32_t position_1indexed = mut_json["position"].get<uint32_t>();
      CHECK_SILO_QUERY(
         position_1indexed > 0,
         "The field 'position' in a {} MutationProfile mutation is 1-indexed. Value 0 is not "
         "allowed.",
         SymbolType::SYMBOL_NAME
      );
      const uint32_t position_idx = position_1indexed - 1;

      const std::string& symbol_str = mut_json["symbol"];
      CHECK_SILO_QUERY(
         symbol_str.size() == 1,
         "The field 'symbol' in a {} MutationProfile mutation must be exactly one character",
         SymbolType::SYMBOL_NAME
      );
      const auto symbol = SymbolType::charToSymbol(symbol_str[0]);
      CHECK_SILO_QUERY(
         symbol.has_value(),
         "Invalid {} symbol '{}' in MutationProfile mutations",
         SymbolType::SYMBOL_NAME,
         symbol_str
      );

      mutations.push_back({position_idx, symbol.value()});
   }

   filter = std::make_unique<MutationProfile<SymbolType>>(
      seq_name, distance, typename MutationProfile<SymbolType>::MutationsInput{std::move(mutations)}
   );
}

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<MutationProfile<Nucleotide>>& filter
);

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<MutationProfile<AminoAcid>>& filter
);

template class MutationProfile<AminoAcid>;
template class MutationProfile<Nucleotide>;

}  // namespace silo::query_engine::filter::expressions
