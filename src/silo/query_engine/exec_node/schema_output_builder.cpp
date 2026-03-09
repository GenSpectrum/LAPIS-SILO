#include "silo/query_engine/exec_node/schema_output_builder.h"

namespace silo::query_engine::exec_node {

SchemaOutputBuilder::SchemaOutputBuilder(std::vector<schema::ColumnIdentifier> schema)
    : schema_(std::move(schema)) {
   for (const auto& output_field : schema_) {
      builders_.emplace(output_field.name, exec_node::columnTypeToArrowType(output_field.type));
   }
}

arrow::Status SchemaOutputBuilder::addValueIfContainedInOutput(
   std::string_view value_name,
   const std::function<std::optional<std::variant<std::string, bool, int32_t, double>>()>& value
) {
   if (auto builder = builders_.find(value_name); builder != builders_.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(value()));
   }
   return arrow::Status::OK();
}

arrow::Result<std::vector<arrow::Datum>> SchemaOutputBuilder::finish() {
   std::vector<arrow::Datum> result;
   result.reserve(schema_.size());
   for (const auto& output_field : schema_) {
      arrow::Datum datum;
      ARROW_ASSIGN_OR_RAISE(datum, builders_.at(output_field.name).toDatum());
      result.push_back(std::move(datum));
   }
   return result;
}

}  // namespace silo::query_engine::exec_node
