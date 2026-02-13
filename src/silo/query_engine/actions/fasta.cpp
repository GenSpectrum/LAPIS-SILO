#include "silo/query_engine/actions/fasta.h"

#include <memory>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

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

namespace {

const std::string SEQUENCE_NAMES_FIELD_NAME = "sequenceNames";
const std::string ADDITIONAL_FIELDS_FIELD_NAME = "additionalFields";

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action) {
   CHECK_SILO_QUERY(
      json.contains(SEQUENCE_NAMES_FIELD_NAME) && json[SEQUENCE_NAMES_FIELD_NAME].is_array(),
      "The Fasta action requires a {} field, which must be an array of strings",
      SEQUENCE_NAMES_FIELD_NAME
   );
   std::vector<std::string> sequence_names;
   for (const auto& child : json[SEQUENCE_NAMES_FIELD_NAME]) {
      CHECK_SILO_QUERY(
         child.is_string(),
         "The Fasta action requires a {} field, which must be an array of "
         "strings; while parsing array encountered the element {} which is not of type string",
         SEQUENCE_NAMES_FIELD_NAME,
         child.dump()
      );
      sequence_names.emplace_back(child.get<std::string>());
   }
   std::vector<std::string> additional_fields;
   if (json.contains(ADDITIONAL_FIELDS_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[ADDITIONAL_FIELDS_FIELD_NAME].is_array(),
         "The field `{}` in a Fasta action must be an array of strings.",
         ADDITIONAL_FIELDS_FIELD_NAME
      );
      for (const auto& child : json[ADDITIONAL_FIELDS_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field `{}` in a Fasta action must be an array of strings. "
            "Encountered non-string element: {}",
            ADDITIONAL_FIELDS_FIELD_NAME,
            child.dump()
         );
         additional_fields.emplace_back(child.get<std::string>());
      }
   }
   action = std::make_unique<Fasta>(
      Action::deduplicateOrderPreserving(sequence_names),
      Action::deduplicateOrderPreserving(additional_fields)
   );
}

}  // namespace silo::query_engine::actions
