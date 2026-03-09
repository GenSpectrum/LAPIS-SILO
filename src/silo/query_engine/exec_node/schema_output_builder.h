#pragma once

#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <arrow/datum.h>
#include <arrow/status.h>

#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::exec_node {

class SchemaOutputBuilder {
   std::vector<schema::ColumnIdentifier> schema_;
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder> builders_;

  public:
   explicit SchemaOutputBuilder(std::vector<schema::ColumnIdentifier> schema);

   arrow::Status addValueIfContainedInOutput(
      std::string_view value_name,
      const std::function<std::optional<std::variant<std::string, bool, int32_t, double>>()>& value
   );

   arrow::Result<std::vector<arrow::Datum>> finish();
};

}  // namespace silo::query_engine::exec_node
