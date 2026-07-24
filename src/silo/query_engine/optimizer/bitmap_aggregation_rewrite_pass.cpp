#include "silo/query_engine/optimizer/bitmap_aggregation_rewrite_pass.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/bitmap_aggregation_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/scalar_expressions/at.h"
#include "silo/query_engine/scalar_expressions/field_ref.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/query_engine/scalar_expressions/zstd_decompress_scalar.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::optimizer {

namespace {

bool isSequenceColumn(const schema::ColumnIdentifier& column) {
   return column.type == schema::ColumnType::NUCLEOTIDE_SEQUENCE ||
          column.type == schema::ColumnType::AMINO_ACID_SEQUENCE;
}

/// The bare `count()` group-by (a single count aggregate, no source column, at least one grouping
/// key) is the only shape this rewrite recognizes.
bool isBareCountGroupBy(const operators::AggregateNode& node) {
   if (node.group_by_fields.empty()) {
      return false;
   }
   return node.aggregates.size() == 1 &&
          node.aggregates[0].function == operators::AggregateFunction::COUNT &&
          !node.aggregates[0].source_column.has_value();
}

/// The map assignment producing `field`, or nullopt if there is no map or none produces it (the key
/// is then a passthrough column read straight from the scan).
std::optional<std::reference_wrapper<const operators::MapNode::Assignment>> findAssignment(
   std::optional<std::reference_wrapper<const operators::MapNode>> map,
   const std::string& field
) {
   if (!map.has_value()) {
      return std::nullopt;
   }
   const auto& assignments = map->get().assignments;
   const auto assignment = std::ranges::find_if(assignments, [&](const auto& candidate) {
      return candidate.output_column.name == field;
   });
   if (assignment == assignments.end()) {
      return std::nullopt;
   }
   return *assignment;
}

struct SequencePositionKey {
   const schema::ColumnIdentifier& column;
   uint32_t position;  // 1-indexed character position
};

/// Matches `at(pos, col)` or `at(pos, zstdDecompress(col))` with `col` a direct column reference,
/// returning the column and position. The zstd-decompress is transparent here: the bitmap engine
/// reads the sequence column's per-symbol bitmaps directly, so a decompressed input matches as a
/// raw one does.
std::optional<SequencePositionKey> asSequencePositionKey(
   const operators::MapNode::Assignment& assignment
) {
   const auto* at =
      scalar_expressions::dynCast<scalar_expressions::At>(assignment.expression.get());
   if (at == nullptr) {
      return std::nullopt;
   }
   const auto* decompress =
      scalar_expressions::dynCast<scalar_expressions::ZstdDecompressScalar>(at->input.get());
   const auto* field_ref = scalar_expressions::dynCast<scalar_expressions::FieldRef>(
      decompress != nullptr ? decompress->input.get() : at->input.get()
   );
   if (field_ref == nullptr) {
      return std::nullopt;
   }
   return SequencePositionKey{.column = field_ref->column, .position = at->position};
}

/// The plan beneath a candidate aggregate: the table scan the grouping reads from, and the single
/// map computing the grouping keys if there is one.
struct GroupBySource {
   // Absent = no map, the aggregate reads straight from the scan. Only inspected, hence const.
   std::optional<std::reference_wrapper<const operators::MapNode>> map;
   // Non-const: the rewrite moves the scan's `table` and `filter` out of it.
   operators::TableScanNode& scan;
};

std::optional<GroupBySource> groupBySource(operators::QueryNode& aggregate_child) {
   const auto* map = dynamic_cast<const operators::MapNode*>(&aggregate_child);

   // The table scan sits below the map when there is one, otherwise directly below the aggregate.
   operators::QueryNode* scan_node = &aggregate_child;
   if (map != nullptr) {
      scan_node = map->child.get();
   }

   auto* scan = dynamic_cast<operators::TableScanNode*>(scan_node);
   if (scan == nullptr) {
      return std::nullopt;
   }
   std::optional<std::reference_wrapper<const operators::MapNode>> map_ref;
   if (map != nullptr) {
      map_ref = *map;
   }
   return GroupBySource{.map = map_ref, .scan = *scan};
}

/// The dimension for `group_by_field` when the map produces it as a sequence-position lookup
/// (`field := seq.at(pos)`), or nullopt otherwise.
std::optional<operators::GroupingDimension> matchSequencePositionDimension(
   const GroupBySource& source,
   const schema::ColumnIdentifier& group_by_field
) {
   const auto assignment = findAssignment(source.map, group_by_field.name);
   if (!assignment.has_value()) {
      return std::nullopt;
   }
   const auto key = asSequencePositionKey(assignment->get());
   if (!key.has_value()) {
      return std::nullopt;
   }
   const auto column = source.scan.table->schema->getColumn(key->column.name);
   if (!column.has_value() || !isSequenceColumn(column.value())) {
      return std::nullopt;
   }
   return operators::SequencePositionDimension{
      column.value(),
      key->position - 1,
      column->type == schema::ColumnType::NUCLEOTIDE_SEQUENCE,
      group_by_field.name
   };
}

/// The dimension for `group_by_field` when it is an indexed string column read straight from the
/// scan (not produced by the map), or nullopt otherwise.
std::optional<operators::GroupingDimension> matchIndexedColumnDimension(
   const GroupBySource& source,
   const schema::ColumnIdentifier& group_by_field
) {
   if (findAssignment(source.map, group_by_field.name).has_value()) {
      return std::nullopt;  // produced by the map, not a plain scan column
   }
   const auto column = source.scan.table->schema->getColumn(group_by_field.name);
   if (!column.has_value() || column->type != schema::ColumnType::INDEXED_STRING) {
      return std::nullopt;
   }
   return operators::IndexedColumnDimension{column.value(), group_by_field.name};
}

/// Resolves one grouping key to its bitmap dimension, or nullopt if no supported dimension matches
/// (which makes the whole rewrite decline). Add a matcher and one `if` to support another shape.
std::optional<operators::GroupingDimension> resolveDimension(
   const GroupBySource& source,
   const schema::ColumnIdentifier& group_by_field
) {
   if (auto dimension = matchSequencePositionDimension(source, group_by_field)) {
      return dimension;
   }
   if (auto dimension = matchIndexedColumnDimension(source, group_by_field)) {
      return dimension;
   }
   return std::nullopt;
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr BitmapAggregationRewritePass::operator()(operators::AggregateNode& node) {
   propagateToNode(node.child);

   if (!isBareCountGroupBy(node)) {
      return nullptr;
   }

   // Whenever the plan is not a shape this rewrite can resolve, decline (nullptr) and leave the
   // generic map/groupBy pipeline to handle it rather than erroring.
   auto source = groupBySource(*node.child);
   if (!source.has_value()) {
      return nullptr;
   }

   std::vector<operators::GroupingDimension> dimensions;
   dimensions.reserve(node.group_by_fields.size());
   for (const auto& group_by_field : node.group_by_fields) {
      auto dimension = resolveDimension(*source, group_by_field);
      if (!dimension.has_value()) {
         return nullptr;
      }
      dimensions.push_back(std::move(dimension.value()));
   }

   return std::make_unique<operators::BitmapAggregationNode>(
      std::move(source->scan.table),
      std::move(source->scan.filter),
      std::move(dimensions),
      node.aggregates[0].output_name
   );
}

}  // namespace silo::query_engine::optimizer
