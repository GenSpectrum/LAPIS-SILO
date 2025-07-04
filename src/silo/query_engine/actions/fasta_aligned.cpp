#include "silo/query_engine/actions/fasta_aligned.h"

#include <iostream>
#include <map>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(
   std::vector<std::string>&& sequence_names,
   std::vector<std::string>&& additional_fields
)
    : sequence_names(sequence_names),
      additional_fields(additional_fields) {}

std::vector<schema::ColumnIdentifier> FastaAligned::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::set<schema::ColumnIdentifier> fields;
   for (const auto& sequence_name : sequence_names) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value() && isSequenceColumn(column.value().type),
         "The table does not contain the SequenceColumn '{}'",
         sequence_name
      );
      fields.emplace(column.value());
   }
   for (const auto& sequence_name : additional_fields) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value(), "The table does not contain the Column '{}'", sequence_name
      );
      fields.emplace(column.value());
   }
   fields.emplace(table_schema.primary_key);
   std::vector<schema::ColumnIdentifier> unique_fields{fields.begin(), fields.end()};
   return unique_fields;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceNames") && json["sequenceNames"].is_array(),
      "The FastaAligned action requires a sequenceNames field, which must be an array of strings"
   );
   std::vector<std::string> sequence_names;
   for (const auto& child : json["sequenceNames"]) {
      CHECK_SILO_QUERY(
         child.is_string(),
         "The FastaAligned action requires a sequenceNames field, which must be an array of "
         "strings; while parsing array encountered the element {} which is not of type string",
         child.dump()
      );
      sequence_names.emplace_back(child.get<std::string>());
   }
   std::vector<std::string> additional_fields;
   if (json.contains("additionalFields")) {
      CHECK_SILO_QUERY(
         json["additionalFields"].is_array(),
         "The field `additionalFields` in a FastaAligned action must be an array of strings."
      );
      for (const auto& child : json["additionalFields"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field `additionalFields` in a FastaAligned action must be an array of strings. "
            "Encountered non-string element: {}",
            child.dump()
         );
         additional_fields.emplace_back(child.get<std::string>());
      }
   }
   action = std::make_unique<FastaAligned>(std::move(sequence_names), std::move(additional_fields));
}

}  // namespace silo::query_engine::actions
