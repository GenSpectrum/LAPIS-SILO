#include "silo/query_engine/operators/map_node.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/datum.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/common/size_constants.h"
#include "silo/query_engine/exec_node/throttled_batch_reslicer.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/query_engine/expressions/field_ref.h"
#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/expressions/zstd_decompress_scalar.h"

namespace silo::query_engine::operators {

namespace {

/// Translates a scalar expression into an Arrow compute expression for use in a
/// projection. Literals and column references are supported; a boolean filter
/// predicate would need to be evaluated against the column store first.
arrow::Result<arrow::compute::Expression> scalarToArrowExpression(
   const expressions::Expression& expression
) {
   if (const auto* literal = dynamic_cast<const expressions::Int64Literal*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::FloatLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::StringLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynamic_cast<const expressions::BoolLiteral*>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* field_ref = dynamic_cast<const expressions::FieldRef*>(&expression)) {
      return arrow::compute::field_ref(field_ref->column.name);
   }
   if (const auto* zstd =
          dynamic_cast<const expressions::ZstdDecompressScalar*>(&expression)) {
      return exec_node::ZstdDecompressExpression::make(
         arrow::compute::field_ref(zstd->input_column.name), zstd->dictionary_string
      );
   }
   return arrow::Status::NotImplemented(
      "non-literal scalar expressions are not yet supported in map() assignments"
   );
}

}  // namespace

MapNode::MapNode(QueryNodePtr child, std::vector<Assignment> assignments)
    : child(std::move(child)),
      assignments(std::move(assignments)) {}

std::vector<schema::ColumnIdentifier> MapNode::getOutputSchema() const {
   auto output = child->getOutputSchema();
   for (const auto& assignment : assignments) {
      auto found = std::ranges::find_if(output, [&](const auto& column) {
         return column.name == assignment.output_column.name;
      });
      if (found != output.end()) {
         *found = assignment.output_column;
      } else {
         output.push_back(assignment.output_column);
      }
   }
   return output;
}

arrow::Result<arrow::acero::ExecNode*> MapNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto* current_node, child->addToExecPlan(plan, tables, query_options));

   // When any assignment uses zstd decompression, insert a backpressure sink/source pair
   // before the projection so that Arrow can throttle the upstream scan appropriately.
   size_t sum_of_reference_genome_sizes = 0;
   for (const auto& assignment : assignments) {
      if (const auto* zstd =
             dynamic_cast<const expressions::ZstdDecompressScalar*>(assignment.expression.get())) {
         sum_of_reference_genome_sizes += zstd->dictionary_string.size();
      }
   }

   if (sum_of_reference_genome_sizes > 0) {
      const auto& input_ordering = current_node->ordering();

      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> batch_generator;
      arrow::acero::BackpressureMonitor* backpressure_monitor;
      std::shared_ptr<arrow::Schema> schema_of_sequence_batches;
      ARROW_ASSIGN_OR_RAISE(
         current_node,
         arrow::acero::MakeExecNode(
            "sink",
            &plan,
            {current_node},
            arrow::acero::SinkNodeOptions{
               &batch_generator,
               &schema_of_sequence_batches,
               arrow::acero::BackpressureOptions{silo::common::S_16_KB, silo::common::S_64_MB},
               &backpressure_monitor
            }
         )
      );
      current_node->SetLabel(
         "additional sink node to help backpressure application before zstd decompression"
      );

      const auto maximum_batch_size = static_cast<int64_t>(
         std::max(silo::common::S_64_MB / sum_of_reference_genome_sizes, 1UL)
      );
      constexpr std::chrono::milliseconds TARGET_BATCH_RATE{667};

      ARROW_ASSIGN_OR_RAISE(
         current_node,
         arrow::acero::MakeExecNode(
            "source",
            &plan,
            {},
            arrow::acero::SourceNodeOptions{
               schema_of_sequence_batches,
               silo::query_engine::exec_node::ThrottledBatchReslicer{
                  batch_generator, maximum_batch_size, TARGET_BATCH_RATE, backpressure_monitor
               },
               input_ordering
            }
         )
      );
      current_node->SetLabel(
         "additional source node to help backpressure application before zstd decompression"
      );
   }

   // Map output column names to the assignment that produces them.
   std::map<std::string, const Assignment*> assignment_by_name;
   for (const auto& assignment : assignments) {
      assignment_by_name[assignment.output_column.name] = &assignment;
   }

   const auto output_schema = getOutputSchema();
   std::vector<arrow::Expression> expressions;
   std::vector<std::string> names;
   expressions.reserve(output_schema.size());
   names.reserve(output_schema.size());
   for (const auto& [name, type] : output_schema) {
      names.push_back(name);
      auto found = assignment_by_name.find(name);
      if (found == assignment_by_name.end()) {
         // Column passed through unchanged from the child.
         expressions.push_back(arrow::compute::field_ref(name));
         continue;
      }
      ARROW_ASSIGN_OR_RAISE(
         auto arrow_expression, scalarToArrowExpression(*found->second->expression)
      );
      expressions.push_back(std::move(arrow_expression));
   }

   const arrow::acero::ProjectNodeOptions options{std::move(expressions), std::move(names)};
   return arrow::acero::MakeExecNode("project", &plan, {current_node}, options);
}

nlohmann::json MapNode::toJson() const {
   nlohmann::json map_expressions = nlohmann::json::array();
   for (const auto& [field, expression] : assignments) {
      map_expressions.push_back(nlohmann::json{field.name, expression->toString()});
   }
   return nlohmann::json{
      {"type", nodeKindToString(kind())},
      {"child", child->toJson()},
      {"mapExpressions", map_expressions}
   };
}

}  // namespace silo::query_engine::operators
