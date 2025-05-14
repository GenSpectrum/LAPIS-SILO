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
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/exec_node/select.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/sequence_column.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(
   std::vector<std::string>&& sequence_names,
   std::vector<std::string>&& additional_fields
)
    : sequence_names(sequence_names),
      additional_fields(additional_fields) {}

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

QueryResult FastaAligned::execute(
   const std::shared_ptr<const storage::Table>& table,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   SILO_PANIC("Legacy execute called on action already migrated action. Programming error.");
}

std::vector<schema::ColumnIdentifier> FastaAligned::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields;
   for (const auto& sequence_name : sequence_names) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(column.has_value(), "Needs to contain X TODO");  // TODO also a sequence name
      fields.emplace_back(column.value());
   }
   for (const auto& sequence_name : additional_fields) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(column.has_value(), "Needs to contain X2 TODO");  // TODO
      fields.emplace_back(column.value());
   }
   fields.push_back(table_schema.primary_key);
   return fields;
}

QueryPlan FastaAligned::toQueryPlan(
   const std::shared_ptr<const storage::Table>& table,
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   std::ostream& output_stream
) {
   QueryPlan query_plan;
   // TODO move to `toExecPlan` method
   // but it will sometimes be more than one exec_node? select -> order -> limit
   std::unique_ptr<arrow::acero::ExecNode> source_node;
   source_node = std::make_unique<exec_node::Select>(
      query_plan.arrow_plan.get(), getOutputSchema(table->schema), partition_filter_operators, table
   );

   std::unique_ptr<arrow::acero::ExecNode> sink_node = std::make_unique<exec_node::NdjsonSink>(
      query_plan.arrow_plan.get(), &output_stream, source_node.get()
   );
   // TODO make configurable std::make_unique<ArrowSinkNode>(query_plan.arrow_plan.get(),
   // &output_stream, source_node.get());

   query_plan.arrow_plan->AddNode(std::move(source_node));
   query_plan.arrow_plan->AddNode(std::move(sink_node));

   return query_plan;
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
   std::vector<std::string> additional_fields;
   if (json.contains("additionalFields") && json["additionalFields"].is_array()) {
      for (const auto& child : json["additionalFields"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "FastaAligned action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         additional_fields.emplace_back(child.get<std::string>());
      }
   }
   action = std::make_unique<FastaAligned>(std::move(sequence_names), std::move(additional_fields));
}

}  // namespace silo::query_engine::actions
