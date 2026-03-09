#include "silo/query_engine/actions/aggregated.h"

#include <optional>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/storage/table.h"

namespace {

const std::string GROUP_BY_FIELDS_FIELD_NAME = "groupByFields";

}  // namespace

namespace silo::query_engine::actions {

Aggregated::Aggregated(std::vector<std::string> group_by_fields_) {
   group_by_fields.reserve(group_by_fields_.size());
   for (auto& field : group_by_fields_) {
      group_by_fields.emplace_back(std::move(field));
   }
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action) {
   std::vector<std::string> group_by_fields;
   if (json.contains(GROUP_BY_FIELDS_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[GROUP_BY_FIELDS_FIELD_NAME].is_array(),
         "{} must be an array",
         GROUP_BY_FIELDS_FIELD_NAME
      );
      for (const auto& element : json[GROUP_BY_FIELDS_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            element.is_string(),
            "{} is not a valid entry in {}. Expected type string, got {}",
            element.dump(),
            GROUP_BY_FIELDS_FIELD_NAME,
            element.type_name()
         );
         group_by_fields.emplace_back(element.get<std::string>());
      }
   }
   action = std::make_unique<Aggregated>(std::move(group_by_fields));
}

}  // namespace silo::query_engine::actions
