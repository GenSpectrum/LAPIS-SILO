#include "silo/query_engine/actions/fasta.h"

#if defined(__linux__)
#include <malloc.h>
#endif

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include <memory>
#include <nlohmann/json.hpp>

#include "silo/common/numbers.h"
#include "silo/common/panic.h"
#include "silo/common/range.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"

using silo::common::add1;
using silo::common::Range;

namespace silo::query_engine::actions {

Fasta::Fasta(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

void Fasta::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::string& primary_key_field = schema.primary_key.name;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::ranges::find(sequence_names, field.name) != std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the Fasta action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      )
   }
}

const size_t PARTITION_CHUNK_SIZE = 100;

QueryResultEntry makeEntry(
   const std::string& primary_key_column,
   const storage::TablePartition& table_partition,
   const std::vector<std::string>& sequence_names,
   const uint32_t row_id
) {
   QueryResultEntry entry;
   entry.fields.emplace(
      primary_key_column, table_partition.columns.getValue(primary_key_column, row_id)
   );
   for (const auto& sequence_name : sequence_names) {
      const auto& column = table_partition.columns.zstd_compressed_string_columns.at(
         storage::UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX + sequence_name
      );
      entry.fields.emplace(sequence_name, column.getDecompressed(row_id));
   }
   return entry;
}

QueryResult Fasta::execute(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   auto columns_in_database =
      table->schema.getColumnByType<storage::column::ZstdCompressedStringColumnPartition>();
   for (const std::string& sequence_name : sequence_names) {
      schema::ColumnIdentifier column_identifier_to_find{
         storage::UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX + sequence_name,
         schema::ColumnType::ZSTD_COMPRESSED_STRING
      };
      CHECK_SILO_QUERY(
         std::ranges::find(columns_in_database, column_identifier_to_find) !=
            columns_in_database.end(),
         "Database does not contain an unaligned sequence with name: '" + sequence_name + "'"
      )
   }

#if defined(__linux__)
   SPDLOG_INFO(
      "Manually invoking malloc_trim() to give "
      "back memory to OS."
   );
   malloc_trim(0);
#endif

   uint32_t partition_index = 0;
   std::optional<Range<uint32_t>> remaining_result_row_indices{};
   return QueryResult::fromGenerator([sequence_names = std::move(sequence_names),
                                      bitmap_filter = std::move(bitmap_filter),
                                      remaining_result_row_indices,
                                      table,
                                      partition_index](std::vector<QueryResultEntry>& results
                                     ) mutable {
      for (; partition_index < table->getNumberOfPartitions();
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
               table->getNumberOfPartitions(),
               result_row_indices.toString(),
               remaining_result_row_indices->beyondLast()
            );

            const auto& database_partition = table->getPartition(partition_index);
            for (const uint32_t row_id : *bitmap) {
               results.emplace_back(makeEntry(
                  table->schema.primary_key.name, database_partition, sequence_names, row_id
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

std::vector<schema::ColumnIdentifier> Fasta::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields;
   for (const auto& sequence_name : sequence_names) {
      // TODO(#763) leave it zstd compressed
      fields.emplace_back(sequence_name, schema::ColumnType::STRING);
   }
   fields.push_back(table_schema.primary_key);
   return fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "Fasta action must have the field sequenceName of type string or an array of "
      "strings"
   );
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "Fasta action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   action = std::make_unique<Fasta>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions
