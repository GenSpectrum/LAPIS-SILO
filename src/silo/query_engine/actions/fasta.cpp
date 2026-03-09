#include "silo/query_engine/actions/fasta.h"

#include <memory>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::actions {

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
   action = std::make_unique<Fasta>(std::move(sequence_names), std::move(additional_fields));
}

}  // namespace silo::query_engine::actions
