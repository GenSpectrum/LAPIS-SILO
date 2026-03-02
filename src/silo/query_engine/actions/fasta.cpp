#include "silo/query_engine/actions/fasta.h"

#include <memory>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::actions {

std::vector<schema::ColumnIdentifier> Fasta::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::set<schema::ColumnIdentifier> fields;
   auto columns_in_database =
      table_schema.getColumnByType<storage::column::ZstdCompressedStringColumnPartition>();
   for (const auto& sequence_name : sequence_names) {
      const schema::ColumnIdentifier column_identifier{
         .name = sequence_name, .type = schema::ColumnType::ZSTD_COMPRESSED_STRING
      };
      CHECK_SILO_QUERY(
         std::ranges::find(columns_in_database, column_identifier) != columns_in_database.end(),
         "Database does not contain an unaligned sequence with name: '{}'",
         sequence_name
      );
      fields.emplace(column_identifier);
   }
   for (const auto& additional_field : additional_fields) {
      auto column = table_schema.getColumn(additional_field);
      CHECK_SILO_QUERY(
         column.has_value(), "The table does not contain the Column '{}'", additional_field
      );
      fields.emplace(column.value());
   }
   fields.emplace(table_schema.primary_key);
   std::vector<schema::ColumnIdentifier> unique_fields{fields.begin(), fields.end()};
   return unique_fields;
}

}  // namespace silo::query_engine::actions
