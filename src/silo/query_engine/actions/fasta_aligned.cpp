#include "silo/query_engine/actions/fasta_aligned.h"

#include <iostream>
#include <map>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <silo/common/numbers.h>
#include <silo/common/range.h>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/sequence_column.h"

using silo::common::add1;
using silo::common::Range;

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

void FastaAligned::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::string& primary_key_field = schema.primary_key.name;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::ranges::find(sequence_names, field.name) != std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the FastaAligned action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      );
   }
}

namespace {
template <typename SymbolType>
std::string reconstructSequence(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_store,
   uint32_t row_id
) {
   std::string reconstructed_sequence;
   std::ranges::transform(
      sequence_store.metadata->reference_sequence,
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
                   position.getBitmap(symbol)->contains(row_id)) {
                  reconstructed_sequence[position_id] = SymbolType::symbolToChar(symbol);
               }
            }
         }
      }
   );

   for (const size_t position_idx : sequence_store.missing_symbol_bitmaps.at(row_id)) {
      reconstructed_sequence[position_idx] = SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
   }
   return reconstructed_sequence;
}

// Note: fasta.cpp has its own PARTITION_CHUNK_SIZE
const size_t PARTITION_CHUNK_SIZE = 100;

/// Split items in sequence_names vector into two vectors
void assortSequenceNamesInto(
   const schema::TableSchema& schema,
   const std::vector<std::string>& sequence_names,
   std::vector<std::string>& nuc_sequence_names,
   std::vector<std::string>& aa_sequence_names
) {
   for (const std::string& sequence_name : sequence_names) {
      auto column = schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value() && (column.value().type == schema::ColumnType::NUCLEOTIDE_SEQUENCE ||
                                column.value().type == schema::ColumnType::AMINO_ACID_SEQUENCE),
         "Database does not contain a sequence with name: '" + sequence_name + "'"
      );
      if (column.value().type == schema::ColumnType::NUCLEOTIDE_SEQUENCE) {
         nuc_sequence_names.emplace_back(sequence_name);
      } else {
         aa_sequence_names.emplace_back(sequence_name);
      }
   }
}

QueryResultEntry makeEntry(
   const std::string& primary_key_column,
   const storage::TablePartition& table_partition,
   const std::vector<std::string>& nuc_sequence_names,
   const std::vector<std::string>& aa_sequence_names,
   const uint32_t row_id
) {
   QueryResultEntry entry;
   entry.fields.emplace(
      primary_key_column, table_partition.columns.getValue(primary_key_column, row_id)
   );
   for (const auto& nuc_sequence_name : nuc_sequence_names) {
      const auto& sequence_store = table_partition.columns.nuc_columns.at(nuc_sequence_name);
      entry.fields.emplace(
         nuc_sequence_name, reconstructSequence<Nucleotide>(sequence_store, row_id)
      );
   }
   for (const auto& aa_sequence_name : aa_sequence_names) {
      const auto& aa_store = table_partition.columns.aa_columns.at(aa_sequence_name);
      entry.fields.emplace(aa_sequence_name, reconstructSequence<AminoAcid>(aa_store, row_id));
   }
   return entry;
}

}  // namespace

QueryResult FastaAligned::execute(
   const Database& database,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

   assortSequenceNamesInto(
      database.table->schema, sequence_names, nuc_sequence_names, aa_sequence_names
   );

   uint32_t partition_index = 0;
   std::optional<Range<uint32_t>> remaining_result_row_indices{};
   return QueryResult::fromGenerator([nuc_sequence_names = std::move(nuc_sequence_names),
                                      aa_sequence_names = std::move(aa_sequence_names),
                                      bitmap_filter = std::move(bitmap_filter),
                                      remaining_result_row_indices,
                                      &database,
                                      partition_index](std::vector<QueryResultEntry>& results
                                     ) mutable {
      for (; partition_index < database.table->getNumberOfPartitions();
           ++partition_index, remaining_result_row_indices = {}) {
         // We drain the bitmaps in bitmap_filter as we process the
         // query, because roaring bitmaps don't come with
         // external, only internal iterators, which can't be used
         // for our external iterator. Instead of implementing an
         // external iterator on bitmaps, we just remove the bitmap
         // members as we process them. To know how far into the
         // result generation we are, we maintain a `Range` of
         // output rows at the same time.
         auto& bitmap = bitmap_filter[partition_index];
         if (!remaining_result_row_indices.has_value()) {
            // We set `remaining_result_row_indices` only once using the
            // original, undrained bitmap.
            remaining_result_row_indices = {
               {0, boost::numeric_cast<uint32_t, uint64_t>(bitmap->cardinality())}
            };
         }

         // The range of results to fully process in this batch
         Range<uint32_t> result_row_indices =
            remaining_result_row_indices->take(PARTITION_CHUNK_SIZE);
         // Remove the same range from the result rows that need to be
         // created for the current partition
         remaining_result_row_indices = remaining_result_row_indices->skip(PARTITION_CHUNK_SIZE);

         if (!result_row_indices.isEmpty()) {
            SPDLOG_TRACE(
               "FastaAligned::execute: refill QueryResult for partition_index {}/{}, {}/{}",
               partition_index,
               database.table->getNumberOfPartitions(),
               result_row_indices.toString(),
               remaining_result_row_indices->beyondLast()
            );

            const auto& database_partition = database.table->getPartition(partition_index);
            for (const uint32_t row_id : *bitmap) {
               results.emplace_back(makeEntry(
                  database.table->schema.primary_key.name,
                  database_partition,
                  nuc_sequence_names,
                  aa_sequence_names,
                  row_id
               ));

               result_row_indices = result_row_indices.skip1();
               if (result_row_indices.isEmpty()) {
                  // Finished the batch. Remove processed `row_id`s;
                  // we already removed the corresponding result
                  // indices from `remaining_result_row_indices`.
                  bitmap->removeRange(0, add1(row_id));
                  // "yield", although control comes back into the
                  // `for` loop from outside:
                  return;
               }
            }
            SILO_PANIC("ran out of bitmap before finishing result_row_indices");
         }
      }
   });
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "FastaAligned action must have the field sequenceName of type string or an array of "
      "strings"
   );
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "FastaAligned action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   action = std::make_unique<FastaAligned>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions
