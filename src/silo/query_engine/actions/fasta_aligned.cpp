#include "silo/query_engine/actions/fasta_aligned.h"

#include "nlohmann/json.hpp"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

#include "silo/common/date.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"

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
            "The only fields returned by the FastaAligned action are {} and {}",
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      )
   }
}

std::string reconstructNucSequence(
   const SequenceStorePartition& sequence_store,
   uint32_t sequence_id
) {
   std::string reconstructed_sequence;
   std::transform(
      sequence_store.reference_genome.begin(),
      sequence_store.reference_genome.end(),
      std::back_inserter(reconstructed_sequence),
      silo::nucleotideSymbolToChar
   );

   for (const auto& [position_id, symbol] :
        sequence_store.indexing_differences_to_reference_genome) {
      reconstructed_sequence[position_id] = nucleotideSymbolToChar(symbol);
   }

   tbb::
      parallel_for(
         tbb::blocked_range<size_t>(0, sequence_store.positions.size()),
         [&](const auto local) {
            for (auto position_id = local.begin(); position_id != local.end(); position_id++) {
               const NucPosition& position = sequence_store.positions.at(position_id);
               for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
                  if (symbol != position.symbol_whose_bitmap_is_flipped &&
                      position.bitmaps.at(symbol).contains(sequence_id)) {
                     reconstructed_sequence[position_id] = nucleotideSymbolToChar(symbol);
                  }
               }
            }
         }
      );

   for (const size_t position : sequence_store.nucleotide_symbol_n_bitmaps.at(sequence_id)) {
      reconstructed_sequence[position] = nucleotideSymbolToChar(NUCLEOTIDE_SYMBOL::N);
   }
   return reconstructed_sequence;
}

std::string reconstructAASequence(const AAStorePartition& aa_store, uint32_t sequence_id) {
   std::string reconstructed_sequence;
   std::transform(
      aa_store.reference_sequence.begin(),
      aa_store.reference_sequence.end(),
      std::back_inserter(reconstructed_sequence),
      silo::aaSymbolToChar
   );

   for (const auto& [position_id, symbol] : aa_store.indexing_differences_to_reference_sequence) {
      reconstructed_sequence[position_id] = aaSymbolToChar(symbol);
   }

   tbb::
      parallel_for(
         tbb::blocked_range<size_t>(0, aa_store.positions.size()),
         [&](const auto local) {
            for (auto position_id = local.begin(); position_id != local.end(); position_id++) {
               const AAPosition& position = aa_store.positions.at(position_id);
               for (const AA_SYMBOL symbol : AA_SYMBOLS) {
                  if (symbol != position.symbol_whose_bitmap_is_flipped &&
                      position.bitmaps.at(symbol).contains(sequence_id)) {
                     reconstructed_sequence[position_id] = aaSymbolToChar(symbol);
                  }
               }
            }
         }
      );

   for (const size_t position : aa_store.aa_symbol_x_bitmaps.at(sequence_id)) {
      reconstructed_sequence[position] = aaSymbolToChar(AA_SYMBOL::X);
   }
   return reconstructed_sequence;
}

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

   QueryResult results;
   for (uint32_t partition_index = 0; partition_index < database.partitions.size();
        ++partition_index) {
      const auto& database_partition = database.partitions[partition_index];
      const auto& bitmap = bitmap_filter[partition_index];
      for (const uint32_t sequence_id : *bitmap) {
         QueryResultEntry entry;
         const std::string primary_key = database.database_config.schema.primary_key;
         entry.fields.emplace(
            primary_key, database_partition.columns.getValue(primary_key, sequence_id)
         );
         for (const auto& nuc_sequence_name : nuc_sequence_names) {
            const auto& sequence_store = database_partition.nuc_sequences.at(nuc_sequence_name);
            entry.fields.emplace(
               nuc_sequence_name, reconstructNucSequence(sequence_store, sequence_id)
            );
         }
         for (const auto& aa_sequence_name : aa_sequence_names) {
            const auto& aa_store = database_partition.aa_sequences.at(aa_sequence_name);
            entry.fields.emplace(aa_sequence_name, reconstructAASequence(aa_store, sequence_id));
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
