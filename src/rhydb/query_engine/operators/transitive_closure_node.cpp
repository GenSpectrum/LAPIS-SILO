#include "rhydb/query_engine/operators/transitive_closure_node.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/array/array_binary.h>
#include <arrow/builder.h>
#include <arrow/compute/exec.h>
#include <arrow/util/async_generator.h>
#include <nlohmann/json.hpp>

#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::operators {

namespace {

/// The interned directed relation: `vertex_names[i]` is the name of vertex `i`, and
/// `adjacency[i]` holds the vertices directly reachable from `i` via a single edge.
struct Relation {
   std::vector<std::string> vertex_names;
   std::vector<std::vector<uint32_t>> adjacency;
};

/// Validates that `column_name` is a STRING column of the input and returns its position, which
/// is also its index into the input's exec batches.
uint32_t validateAndFindColumn(
   const std::vector<schema::ColumnIdentifier>& child_schema,
   const std::string& column_name
) {
   const auto found = std::ranges::find_if(child_schema, [&](const auto& column) {
      return column.name == column_name;
   });
   CHECK_SILO_QUERY(
      found != child_schema.end(),
      "transitiveClosure() column '{}' is not present in the input's output schema",
      column_name
   );
   CHECK_SILO_QUERY(
      found->type == schema::ColumnType::STRING,
      "transitiveClosure() can only be applied to STRING columns, but column '{}' has type {}",
      column_name,
      schema::columnTypeToString(found->type)
   );
   return static_cast<uint32_t>(std::distance(child_schema.begin(), found));
}

arrow::Status appendStringColumn(
   const arrow::Datum& datum,
   std::vector<std::optional<std::string>>& out
) {
   if (!datum.is_array()) {
      return arrow::Status::Invalid("transitiveClosure() expected an array-typed input column");
   }
   const auto array = datum.make_array();
   const auto& string_array = static_cast<const arrow::StringArray&>(*array);
   for (int64_t row = 0; row < string_array.length(); ++row) {
      if (string_array.IsNull(row)) {
         out.emplace_back(std::nullopt);
      } else {
         out.emplace_back(string_array.GetString(row));
      }
   }
   return arrow::Status::OK();
}

/// Builds the interned relation from the child's exec batches, reading the edge endpoints from
/// the given column indices. Rows with a null endpoint are skipped.
arrow::Result<Relation> buildRelation(
   const std::vector<std::optional<arrow::ExecBatch>>& batches,
   uint32_t from_index,
   uint32_t to_index
) {
   std::vector<std::optional<std::string>> from_values;
   std::vector<std::optional<std::string>> to_values;
   for (const auto& batch : batches) {
      if (!batch.has_value()) {
         continue;
      }
      ARROW_RETURN_NOT_OK(appendStringColumn(batch->values[from_index], from_values));
      ARROW_RETURN_NOT_OK(appendStringColumn(batch->values[to_index], to_values));
   }

   std::vector<std::string> vertex_names;
   std::unordered_map<std::string, uint32_t> vertex_ids;
   const auto intern = [&](const std::string& value) {
      const auto [iterator, inserted] =
         vertex_ids.try_emplace(value, static_cast<uint32_t>(vertex_names.size()));
      if (inserted) {
         vertex_names.push_back(iterator->first);
      }
      return iterator->second;
   };

   std::vector<std::pair<uint32_t, uint32_t>> edges;
   for (size_t row = 0; row < from_values.size(); ++row) {
      if (!from_values[row].has_value() || !to_values[row].has_value()) {
         continue;
      }
      edges.emplace_back(intern(*from_values[row]), intern(*to_values[row]));
   }

   std::vector<std::vector<uint32_t>> adjacency(vertex_names.size());
   for (const auto& [from, to] : edges) {
      adjacency[from].push_back(to);
   }
   return Relation{.vertex_names = std::move(vertex_names), .adjacency = std::move(adjacency)};
}

/// Computes the transitive closure of `relation`, returning the reachable pairs sorted by
/// (from, to) name so the output is deterministic. If `include_vertices` is set, the reflexive
/// pair (v, v) is added for every vertex.
std::vector<std::pair<std::string, std::string>> computeClosure(
   const Relation& relation,
   bool include_vertices
) {
   const auto num_vertices = static_cast<uint32_t>(relation.vertex_names.size());
   std::vector<std::pair<std::string, std::string>> pairs;

   for (uint32_t source = 0; source < num_vertices; ++source) {
      std::vector<bool> reached(num_vertices, false);
      std::queue<uint32_t> to_visit;
      for (const uint32_t successor : relation.adjacency[source]) {
         if (!reached[successor]) {
            reached[successor] = true;
            to_visit.push(successor);
         }
      }
      while (!to_visit.empty()) {
         const uint32_t current = to_visit.front();
         to_visit.pop();
         for (const uint32_t successor : relation.adjacency[current]) {
            if (!reached[successor]) {
               reached[successor] = true;
               to_visit.push(successor);
            }
         }
      }
      for (uint32_t destination = 0; destination < num_vertices; ++destination) {
         if (reached[destination]) {
            pairs.emplace_back(relation.vertex_names[source], relation.vertex_names[destination]);
         }
      }
      // Add the reflexive pair unless the vertex already reaches itself through a cycle.
      if (include_vertices && !reached[source]) {
         pairs.emplace_back(relation.vertex_names[source], relation.vertex_names[source]);
      }
   }

   std::ranges::sort(pairs);
   return pairs;
}

arrow::Result<std::optional<arrow::ExecBatch>> buildClosureBatch(
   const std::vector<std::pair<std::string, std::string>>& pairs
) {
   arrow::StringBuilder from_builder{};
   arrow::StringBuilder to_builder{};
   for (const auto& [from, to] : pairs) {
      ARROW_RETURN_NOT_OK(from_builder.Append(from));
      ARROW_RETURN_NOT_OK(to_builder.Append(to));
   }
   arrow::Datum from_datum;
   ARROW_ASSIGN_OR_RAISE(from_datum, from_builder.Finish());
   arrow::Datum to_datum;
   ARROW_ASSIGN_OR_RAISE(to_datum, to_builder.Finish());
   ARROW_ASSIGN_OR_RAISE(auto batch, arrow::ExecBatch::Make({from_datum, to_datum}));
   return std::optional<arrow::ExecBatch>{std::move(batch)};
}

}  // namespace

TransitiveClosureNode::TransitiveClosureNode(
   QueryNodePtr child,
   std::string from_column,
   std::string to_column,
   bool include_vertices
)
    : child(std::move(child)),
      from_column(std::move(from_column)),
      to_column(std::move(to_column)),
      include_vertices(include_vertices) {}

std::vector<schema::ColumnIdentifier> TransitiveClosureNode::getOutputSchema() const {
   return {
      {.name = std::string{FROM_COLUMN}, .type = schema::ColumnType::STRING},
      {.name = std::string{TO_COLUMN}, .type = schema::ColumnType::STRING},
   };
}

arrow::Result<arrow::acero::ExecNode*> TransitiveClosureNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   const auto child_schema = child->getOutputSchema();
   const uint32_t from_index = validateAndFindColumn(child_schema, from_column);
   const uint32_t to_index = validateAndFindColumn(child_schema, to_column);

   ARROW_ASSIGN_OR_RAISE(auto* child_node, child->addToExecPlan(plan, tables, query_options));

   // The child runs within this same plan. A sink drains its batches into `child_generator`;
   // the source node below collects them, computes the closure, and emits the result
   // downstream. This is the same sink -> generator -> source shape that orderBy() uses.
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> child_generator;
   ARROW_RETURN_NOT_OK(
      arrow::acero::MakeExecNode(
         "sink", &plan, {child_node}, arrow::acero::SinkNodeOptions{&child_generator}
      )
         .status()
   );

   const bool include_vertices_copy = include_vertices;
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [child_generator = std::move(child_generator),
       from_index,
       to_index,
       include_vertices_copy,
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> end_of_stream = std::nullopt;
         return arrow::Future{end_of_stream};
      }
      already_produced = true;
      return arrow::CollectAsyncGenerator(child_generator)
         .Then(
            [from_index, to_index, include_vertices_copy](
               const std::vector<std::optional<arrow::ExecBatch>>& batches
            ) -> arrow::Result<std::optional<arrow::ExecBatch>> {
               ARROW_ASSIGN_OR_RAISE(
                  const Relation relation, buildRelation(batches, from_index, to_index)
               );
               return buildClosureBatch(computeClosure(relation, include_vertices_copy));
            }
         );
   };

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(getOutputSchema()),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", &plan, {}, options);
}

nlohmann::json TransitiveClosureNode::toJson() const {
   return {
      {"type", nodeKindToString(kind())},
      {"child", child->toJson()},
      {"from", from_column},
      {"to", to_column},
      {"includeVertices", include_vertices},
   };
}

}  // namespace rhydb::query_engine::operators
