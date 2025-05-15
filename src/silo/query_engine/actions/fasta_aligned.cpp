#include "silo/query_engine/actions/fasta_aligned.h"

#include <iostream>
#include <map>
#include <optional>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
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
#include "silo/query_engine/exec_node/table_scan.h"
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
   std::shared_ptr<const storage::Table> table,
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
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   std::ostream& output_stream,
   const config::QueryOptions& query_options
) {
   QueryPlan query_plan;
   arrow::acero::ExecNode* node = query_plan.arrow_plan->EmplaceNode<exec_node::TableScan>(
      query_plan.arrow_plan.get(), getOutputSchema(table->schema), partition_filter_operators, table
   );

   if (auto ordering = getOrdering()) {
      // Create an OrderByNode and put it on top, then replace `node` with the created OrderBy
      auto status = arrow::acero::MakeExecNode(
                       std::string{arrow::acero::OrderByNodeOptions::kName},
                       query_plan.arrow_plan.get(),
                       {node},
                       arrow::acero::OrderByNodeOptions{ordering.value()}
      )
                       .Value(&node);
      if (!status.ok()) {
         SILO_PANIC("Arrow error: {}", status.ToString());
      }
   }
   if (limit.has_value() || offset.has_value()) {
      // Create a FetchNode and put it on top, then replace `node` with the created FetchNode
      arrow::acero::FetchNodeOptions fetch_options(offset.value_or(0), limit.value_or(UINT32_MAX));
      auto status = arrow::acero::MakeExecNode(
                       std::string{arrow::acero::FetchNodeOptions::kName},
                       query_plan.arrow_plan.get(),
                       {node},
                       fetch_options
      )
                       .Value(&node);
      if (!status.ok()) {
         SILO_PANIC("Arrow error: {}", status.ToString());
      }
   }

   // TODO(#764) make output format configurable
   query_plan.arrow_plan->EmplaceNode<exec_node::NdjsonSink>(
      query_plan.arrow_plan.get(), &output_stream, node
   );

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
