#include "silo/query_engine/optimizer/bitmap_aggregation_rewrite_pass.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/bitmap_aggregation_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/scalar_expressions/at.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/query_engine/scalar_expressions/zstd_decompress_scalar.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::optimizer {

namespace {

bool isSequenceColumn(const schema::ColumnIdentifier& column) {
   return column.type == schema::ColumnType::NUCLEOTIDE_SEQUENCE ||
          column.type == schema::ColumnType::AMINO_ACID_SEQUENCE;
}

/// Whether the aggregate is the bare `count()` (a single count aggregate with no source column)
/// with at least one grouping key, the only shape this rewrite recognizes.
bool isBareCountGroupBy(const operators::AggregateNode& node) {
   if (node.group_by_fields.empty()) {
      return false;
   }
   return node.aggregates.size() == 1 &&
          node.aggregates[0].function == operators::AggregateFunction::COUNT &&
          !node.aggregates[0].source_column.has_value();
}

/// If a directly-preceding `map` assigns `field` an `At` expression, return that `At`; otherwise
/// nullptr. `map_node` may be null (no map before the groupBy).
const scalar_expressions::At* findAtAssignment(
   const operators::MapNode* map_node,
   const std::string& field
) {
   if (map_node == nullptr) {
      return nullptr;
   }
   const auto assignment = std::ranges::find_if(map_node->assignments, [&](const auto& candidate) {
      return candidate.output_column.name == field;
   });
   if (assignment == map_node->assignments.end()) {
      return nullptr;
   }
   return dynamic_cast<const scalar_expressions::At*>(assignment->expression.get());
}

/// Whether every assignment of `map_node` merely zstd-decompresses a sequence column. Only such a
/// map is the implicit decompress map that this rewrite may transparently skip; a user-defined map
/// can override arbitrary column values/types, so skipping it would read the wrong inputs.
bool isDecompressMap(const operators::MapNode& map_node) {
   return std::ranges::all_of(map_node.assignments, [](const auto& assignment) {
      return scalar_expressions::isA<scalar_expressions::ZstdDecompressScalar>(
         assignment.expression.get()
      );
   });
}

/// The leaf table scan the aggregate reads from. `map_node` (if any) is the map defining the `At`
/// grouping keys; below it there may be a decompress `MapNode` that remaps sequence columns to
/// strings. Returns nullptr if the aggregate is not (directly or through those maps) reading a
/// single table scan.
operators::TableScanNode* getTableScan(
   operators::QueryNode* aggregate_child,
   const operators::MapNode* map_node
) {
   operators::QueryNode* below = map_node != nullptr ? map_node->child.get() : aggregate_child;
   if (auto* decompress = dynamic_cast<operators::MapNode*>(below);
       decompress != nullptr && isDecompressMap(*decompress)) {
      below = decompress->child.get();
   }
   return dynamic_cast<operators::TableScanNode*>(below);
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr BitmapAggregationRewritePass::operator()(operators::AggregateNode& node) {
   propagateToNode(node.child);

   if (!isBareCountGroupBy(node)) {
      return nullptr;
   }

   // The map (if any) that defines the `At` grouping keys sits directly above the aggregate.
   const auto* map_node = dynamic_cast<const operators::MapNode*>(node.child.get());
   auto* scan = getTableScan(node.child.get(), map_node);
   // An optimizer must never turn a valid query into an error: whenever the plan is not in a shape
   // this rewrite can resolve, decline (return nullptr) and leave the generic map/groupBy pipeline
   // to handle it.
   if (scan == nullptr) {
      return nullptr;
   }

   std::vector<std::unique_ptr<operators::GroupingDimension>> dimensions;
   dimensions.reserve(node.group_by_fields.size());
   for (const auto& group_by_field : node.group_by_fields) {
      if (const auto* at = findAtAssignment(map_node, group_by_field.name)) {
         // A sequence-position grouping key. The `At` reads a column that keeps the sequence
         // column's name (a decompress map only remaps its type to STRING), so resolving that name
         // against the table schema tells us whether it is really a sequence column.
         const auto column = scan->table->schema->getColumn(at->input_column.name);
         if (!column.has_value() || !isSequenceColumn(column.value())) {
            return nullptr;
         }
         dimensions.push_back(std::make_unique<operators::SequencePositionDimension>(
            column.value(),
            at->position - 1,
            column->type == schema::ColumnType::NUCLEOTIDE_SEQUENCE,
            group_by_field.name
         ));
         continue;
      }
      // Not an `At`: a map that computes this key with any other expression is not something the
      // bitmap engine can group on, so decline rather than misread it as a passthrough column.
      if (map_node != nullptr &&
          std::ranges::any_of(map_node->assignments, [&](const auto& assignment) {
             return assignment.output_column.name == group_by_field.name;
          })) {
         return nullptr;
      }
      // A plain table column: only indexed string columns can be grouped straight from their index.
      const auto column = scan->table->schema->getColumn(group_by_field.name);
      if (!column.has_value() || column->type != schema::ColumnType::INDEXED_STRING) {
         return nullptr;
      }
      dimensions.push_back(
         std::make_unique<operators::IndexedColumnDimension>(column.value(), group_by_field.name)
      );
   }

   return std::make_unique<operators::BitmapAggregationNode>(
      std::move(scan->table),
      std::move(scan->filter),
      std::move(dimensions),
      node.aggregates[0].output_name
   );
}

}  // namespace silo::query_engine::optimizer
