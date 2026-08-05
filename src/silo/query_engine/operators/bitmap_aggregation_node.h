#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <arrow/result.h>
#include <nlohmann/json_fwd.hpp>
#include <roaring/roaring.hh>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// The partition of a filtered row-set produced by one grouping dimension: one bitmap per distinct
/// value that actually occurs, keyed by that value's string rendering, plus one final bitmap for
/// the rows that carry no value in this dimension (a null group, keyed by `std::nullopt`). Every
/// group is already intersected with the query filter, so the bitmaps — and hence all downstream
/// work — are bounded by the filtered row set rather than the whole table. Together the groups are
/// disjoint and cover every filtered row. The value is rendered as a string because the aggregation
/// node emits every grouping column as STRING (a null group becomes a SQL null).
using GroupBitmaps = std::vector<std::pair<std::optional<std::string>, roaring::Roaring>>;

/// Groups rows by the symbol they carry at a fixed sequence position, e.g. `main.at(123)`.
struct SequencePositionDimension {
   schema::ColumnIdentifier column;
   uint32_t position_idx;  // 0-based
   bool is_nucleotide;
   std::string output_name;  // e.g. "main[1]"

   SequencePositionDimension(
      schema::ColumnIdentifier column,
      uint32_t position_idx,
      bool is_nucleotide,
      std::string output_name
   );

   /// Partition `filter_bitmap` into this dimension's disjoint, filter-bounded groups (see
   /// `GroupBitmaps`), reading the relevant column from `table`.
   [[nodiscard]] GroupBitmaps buildGroups(
      const storage::Table& table,
      const CopyOnWriteBitmap& filter_bitmap
   ) const;

   /// The STRING output column this dimension contributes to the result schema.
   [[nodiscard]] schema::ColumnIdentifier outputColumn() const;

   [[nodiscard]] nlohmann::json toJson() const;
};

/// Groups rows by the value of an indexed string column, straight from its inverted index.
struct IndexedColumnDimension {
   schema::ColumnIdentifier column;
   std::string output_name;

   IndexedColumnDimension(schema::ColumnIdentifier column, std::string output_name);

   [[nodiscard]] GroupBitmaps buildGroups(
      const storage::Table& table,
      const CopyOnWriteBitmap& filter_bitmap
   ) const;

   [[nodiscard]] schema::ColumnIdentifier outputColumn() const;

   [[nodiscard]] nlohmann::json toJson() const;
};

/// One grouping dimension of a `BitmapAggregationNode`: a rule for partitioning a filtered row-set
/// into disjoint, value-keyed groups directly from roaring bitmaps, plus the STRING output column
/// it contributes. A variant over the supported kinds lets a single query group on a mix of them;
/// add an alternative to support another kind. Every alternative offers `buildGroups`,
/// `outputColumn` and `toJson`, so a generic `std::visit` dispatches over them.
using GroupingDimension = std::variant<SequencePositionDimension, IndexedColumnDimension>;

/// Resolved bitmap-aggregation operator. Groups the rows matched by `filter` by a set of
/// `GroupingDimension`s, emitting one row per observed combination of values together with the
/// number of rows carrying it.
///
/// It is computed by recursively partitioning the filtered row-set with the per-dimension,
/// per-value roaring bitmaps, pruning empty combinations. Only non-empty combinations are visited
/// (their number is bounded by the count of matching rows), so this scales to many dimensions
/// without the exponential blow-up of a full Cartesian product.
class BitmapAggregationNode final : public QueryNode {
  public:
   std::shared_ptr<storage::Table> table;
   std::unique_ptr<scalar_expressions::ScalarExpression> filter;
   std::vector<GroupingDimension> dimensions;
   std::string count_field_name;

   BitmapAggregationNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<scalar_expressions::ScalarExpression> filter,
      std::vector<GroupingDimension> dimensions,
      std::string count_field_name
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& plan,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::BITMAP_AGGREGATION; }

   [[nodiscard]] nlohmann::json toJson() const override;
};

}  // namespace silo::query_engine::operators
