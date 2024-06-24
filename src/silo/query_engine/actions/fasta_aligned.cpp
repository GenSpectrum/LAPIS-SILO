#include "silo/query_engine/actions/fasta_aligned.h"

#include <map>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/sequence_store.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

void FastaAligned::validateOrderByFields(const Database& database) const {
   const std::string& primary_key_field = database.database_config.schema.primary_key;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::find(sequence_names.begin(), sequence_names.end(), field.name) !=
               std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the FastaAligned action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      )
   }
}

namespace {
template <typename SymbolType>
std::string reconstructSequence(
   const SequenceStorePartition<SymbolType>& sequence_store,
   uint32_t sequence_id
) {
   std::string reconstructed_sequence;
   std::transform(
      sequence_store.reference_sequence.begin(),
      sequence_store.reference_sequence.end(),
      std::back_inserter(reconstructed_sequence),
      SymbolType::symbolToChar
   );

   for (const auto& [position_id, symbol] :
        sequence_store.indexing_differences_to_reference_sequence) {
      reconstructed_sequence[position_id] = SymbolType::symbolToChar(symbol);
   }

   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, sequence_store.positions.size()),
      [&](const auto local) {
         for (auto position_id = local.begin(); position_id != local.end(); position_id++) {
            const Position<SymbolType>& position = sequence_store.positions.at(position_id);
            for (const auto symbol : SymbolType::SYMBOLS) {
               if (!position.isSymbolFlipped(symbol) && !position.isSymbolDeleted(symbol) &&
                   position.getBitmap(symbol)->contains(sequence_id)) {
                  reconstructed_sequence[position_id] = SymbolType::symbolToChar(symbol);
               }
            }
         }
      }
   );

   for (const size_t position_idx : sequence_store.missing_symbol_bitmaps.at(sequence_id)) {
      reconstructed_sequence[position_idx] = SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
   }
   return reconstructed_sequence;
}
}  // namespace

QueryResult FastaAligned::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

   for (const std::string& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         database.nuc_sequences.contains(sequence_name) ||
            database.aa_sequences.contains(sequence_name),
         "Database does not contain a sequence with name: '" + sequence_name + "'"
      )
      if (database.nuc_sequences.contains(sequence_name)) {
         nuc_sequence_names.emplace_back(sequence_name);
      } else {
         aa_sequence_names.emplace_back(sequence_name);
      }
   }

   size_t total_count = 0;
   for (auto& filter : bitmap_filter) {
      total_count += filter->cardinality();
   }
   CHECK_SILO_QUERY(total_count < 10001, "FastaAligned action currently limited to 10000 sequences")

   QueryResult results;
   for (uint32_t partition_index = 0; partition_index < database.partitions.size();
        ++partition_index) {
      const auto& database_partition = database.partitions[partition_index];
      const auto& bitmap = bitmap_filter[partition_index];
      for (const uint32_t sequence_id : *bitmap) {
         QueryResultEntry entry;
         const std::string primary_key_column = database.database_config.schema.primary_key;
         entry.fields.emplace(
            primary_key_column, database_partition.columns.getValue(primary_key_column, sequence_id)
         );
         for (const auto& nuc_sequence_name : nuc_sequence_names) {
            const auto& sequence_store = database_partition.nuc_sequences.at(nuc_sequence_name);
            entry.fields.emplace(
               nuc_sequence_name, reconstructSequence<Nucleotide>(sequence_store, sequence_id)
            );
         }
         for (const auto& aa_sequence_name : aa_sequence_names) {
            const auto& aa_store = database_partition.aa_sequences.at(aa_sequence_name);
            entry.fields.emplace(
               aa_sequence_name, reconstructSequence<AminoAcid>(aa_store, sequence_id)
            );
         }
         results.query_result.emplace_back(entry);
      }
   }
   return results;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "FastaAligned action must have the field sequenceName of type string or an array of "
      "strings"
   )
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "FastaAligned action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         )
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   action = std::make_unique<FastaAligned>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions
