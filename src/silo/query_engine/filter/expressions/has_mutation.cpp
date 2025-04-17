#include "silo/query_engine/filter/expressions/has_mutation.h"

#include <map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/symbol_equals.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
HasMutation<SymbolType>::HasMutation(
   std::optional<std::string> sequence_name,
   uint32_t position_idx
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx) {}

template <typename SymbolType>
std::string HasMutation<SymbolType>::toString() const {
   const std::string sequence_name_prefix = sequence_name ? sequence_name.value() + ":" : "";
   return sequence_name_prefix + std::to_string(position_idx);
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> HasMutation<SymbolType>::compile(
   const silo::Database& database,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || database.table->schema.getDefaultSequenceName<SymbolType>(),
      fmt::format(
         "Database does not have a default sequence name for {} Sequences. "
         "You need to provide the sequence name with the {}Mutation filter.",
         SymbolType::SYMBOL_NAME,
         SymbolType::SYMBOL_NAME
      )
   );

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, database.table->schema);

   const auto& seq_store_partition =
      table_partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   auto column_metadata =
      database.table->schema.getColumnMetadata<typename SymbolType::Column>(valid_sequence_name)
         .value();
   CHECK_SILO_QUERY(
      position_idx < column_metadata->reference_sequence.size(),
      fmt::format(
         "Has{}Mutation position is out of bounds {} > {}",
         SymbolType::SYMBOL_NAME,
         position_idx + 1,
         seq_store_partition.metadata->reference_sequence.size()
      )
   )

   auto ref_symbol = seq_store_partition.metadata->reference_sequence.at(position_idx);

   std::vector<typename SymbolType::Symbol> symbols =
      std::vector(SymbolType::SYMBOLS.begin(), SymbolType::SYMBOLS.end());
   if (mode == AmbiguityMode::UPPER_BOUND) {
      // We can only be sure, that the symbol did not mutate, if the ref_symbol is at that position
      std::erase(symbols, ref_symbol);
   } else {
      // Remove all symbols that could match the searched base
      for (const auto symbol : SymbolType::AMBIGUITY_SYMBOLS.at(ref_symbol)) {
         std::erase(symbols, symbol);
      }
   }
   std::vector<std::unique_ptr<Expression>> symbol_filters;
   std::ranges::transform(
      symbols,
      std::back_inserter(symbol_filters),
      [&](typename SymbolType::Symbol symbol) {
         return std::make_unique<SymbolEquals<SymbolType>>(
            valid_sequence_name, position_idx, symbol
         );
      }
   );
   return Or(std::move(symbol_filters)).compile(database, table_partition, NONE);
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasMutation<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.contains("position"),
      fmt::format(
         "The field 'position' is required in a Has{}Mutation expression", SymbolType::SYMBOL_NAME
      )
   );
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      fmt::format(
         "The field 'position' in a Has{}Mutation expression needs to be an unsigned integer",
         SymbolType::SYMBOL_NAME
      )
   );
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position_idx_1_indexed = json["position"].get<uint32_t>();
   CHECK_SILO_QUERY(
      position_idx_1_indexed > 0, "The field 'position' is 1-indexed. Value of 0 not allowed."
   );
   const uint32_t position_idx = position_idx_1_indexed - 1;
   filter = std::make_unique<HasMutation<SymbolType>>(nuc_sequence_name, position_idx);
}

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<HasMutation<Nucleotide>>& filter
);

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<HasMutation<AminoAcid>>& filter
);

template class HasMutation<AminoAcid>;
template class HasMutation<Nucleotide>;

}  // namespace silo::query_engine::filter::expressions
