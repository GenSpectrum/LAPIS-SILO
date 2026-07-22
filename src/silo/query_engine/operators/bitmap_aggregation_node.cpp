#include "silo/query_engine/operators/bitmap_aggregation_node.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/operators/compute_filter.h"
#include "silo/query_engine/scalar_expressions/symbol_in_set.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

namespace {

/// One observed combination across the grouping dimensions together with the number of (filtered)
/// rows carrying it. Each entry of `group_indices` indexes into the corresponding dimension's
/// `GroupBitmaps`, i.e. it identifies which group was chosen in that dimension. Keeping indices
/// rather than the values themselves makes the recursion type-agnostic: the actual value is
/// resolved per dimension only when the output is materialized.
struct GroupCombination {
   std::vector<size_t> group_indices;
   uint64_t count;
};

/// Partitions the sequences of `filter_bitmap` by the symbol they carry at `position_idx`,
/// returning one bitmap per occurring symbol and a final null group for the absent sequences. Each
/// group is intersected with `filter_bitmap`, so the returned bitmaps — and hence all downstream
/// work — are bounded by the filtered row set rather than the whole table, and symbols that no
/// filtered sequence carries are dropped. Reuses the SymbolInSet filter machinery so that
/// reference, missing (no coverage) and ambiguity handling match `SymbolInSet` exactly; the null
/// group restores the null handling that `at()` (the expression this operator replaces) applies but
/// `SymbolInSet` does not, so the grouping matches the generic `at()`/groupBy path.
template <typename SymbolType>
GroupBitmaps buildSymbolBitmaps(
   const storage::column::SequenceColumn<SymbolType>& column,
   uint32_t position_idx,
   const storage::column::RowLayout& row_layout,
   const roaring::Roaring& filter_bitmap
) {
   GroupBitmaps result;
   for (const auto symbol : SymbolType::SYMBOLS) {
      auto compiled = scalar_expressions::compileSymbolInSet<SymbolType>(
         column, position_idx, std::vector<typename SymbolType::Symbol>{symbol}, row_layout
      );
      roaring::Roaring bitmap = compiled->evaluate().getConstReference();
      // Restrict each group to the filtered set: the returned bitmaps and every downstream
      // intersection then scale with the filter rather than the whole table, and symbols that no
      // filtered sequence carries drop out (fewer partition branches).
      bitmap &= filter_bitmap;
      if (!bitmap.isEmpty()) {
         result.emplace_back(std::string(1, SymbolType::symbolToChar(symbol)), std::move(bitmap));
      }
   }
   // The per-symbol filters exclude sequences that are absent at this position (`SymbolInSet` does
   // not treat a null sequence as the missing symbol), so add them back as their own null group.
   // This keeps the groups a partition of the filtered rows and mirrors how the generic
   // `at()`/groupBy path emits a null key for such rows. Appended last so the depth-first output
   // order stays deterministic with the null group after every symbol.
   roaring::Roaring null_group = column.null_bitmap & filter_bitmap;
   if (!null_group.isEmpty()) {
      result.emplace_back(std::nullopt, std::move(null_group));
   }
   return result;
}

/// Recursively partition `current` by the groups of each successive dimension, pruning empty
/// branches. At the leaf one combination (the index of the chosen group per dimension) with its
/// row count is recorded. The recursion never inspects the group values themselves, only the
/// bitmaps and their indices, so it is agnostic to the kind of each dimension.
// NOLINTNEXTLINE(misc-no-recursion)
void partition(
   const roaring::Roaring& current,
   size_t depth,
   const std::vector<GroupBitmaps>& group_bitmaps_per_dimension,
   std::vector<size_t>& accumulated_indices,
   std::vector<GroupCombination>& combinations
) {
   if (depth == group_bitmaps_per_dimension.size()) {
      combinations.push_back(
         GroupCombination{.group_indices = accumulated_indices, .count = current.cardinality()}
      );
      return;
   }
   const auto& dimension = group_bitmaps_per_dimension[depth];
   for (size_t group_index = 0; group_index < dimension.size(); ++group_index) {
      roaring::Roaring intersection = current;
      intersection &= dimension[group_index].second;
      if (intersection.isEmpty()) {
         continue;
      }
      accumulated_indices.push_back(group_index);
      partition(
         intersection, depth + 1, group_bitmaps_per_dimension, accumulated_indices, combinations
      );
      accumulated_indices.pop_back();
   }
}

/// Recursively partitions `filter_bitmap` by the per-dimension group bitmaps, pruning empty
/// branches, and records one combination of values with its row count per surviving leaf. Only
/// non-empty combinations are visited (their number is bounded by the count of matching rows), so
/// this scales to many dimensions without the exponential blow-up of a full Cartesian product.
std::vector<GroupCombination> computeCombinations(
   const roaring::Roaring& filter_bitmap,
   const std::vector<GroupBitmaps>& group_bitmaps_per_dimension
) {
   std::vector<GroupCombination> combinations;
   std::vector<size_t> accumulated_indices;
   partition(filter_bitmap, 0, group_bitmaps_per_dimension, accumulated_indices, combinations);
   return combinations;
}

}  // namespace

SequencePositionDimension::SequencePositionDimension(
   schema::ColumnIdentifier column,
   uint32_t position_idx,
   bool is_nucleotide,
   std::string output_name
)
    : column(std::move(column)),
      position_idx(position_idx),
      is_nucleotide(is_nucleotide),
      output_name(std::move(output_name)) {}

GroupBitmaps SequencePositionDimension::buildGroups(
   const storage::Table& table,
   const roaring::Roaring& filter_bitmap
) const {
   if (is_nucleotide) {
      const auto& sequence_column = table.columns.getColumns<Nucleotide::Column>().at(column.name);
      return buildSymbolBitmaps<Nucleotide>(
         sequence_column, position_idx, table.row_layout, filter_bitmap
      );
   }
   const auto& sequence_column = table.columns.getColumns<AminoAcid::Column>().at(column.name);
   return buildSymbolBitmaps<AminoAcid>(
      sequence_column, position_idx, table.row_layout, filter_bitmap
   );
}

schema::ColumnIdentifier SequencePositionDimension::outputColumn() const {
   return {output_name, schema::ColumnType::STRING};
}

nlohmann::json SequencePositionDimension::toJson() const {
   return {
      {"kind", "sequencePosition"},
      {"column", columnToJson(column)},
      {"position", position_idx},
      {"isNucleotide", is_nucleotide},
      {"outputName", output_name},
   };
}

IndexedColumnDimension::IndexedColumnDimension(
   schema::ColumnIdentifier column,
   std::string output_name
)
    : column(std::move(column)),
      output_name(std::move(output_name)) {}

GroupBitmaps IndexedColumnDimension::buildGroups(
   const storage::Table& table,
   const roaring::Roaring& filter_bitmap
) const {
   const auto& indexed_column =
      table.columns.getColumns<storage::column::IndexedStringColumn>().at(column.name);
   GroupBitmaps result;
   // One group per distinct value in the inverted index. A null row is held in `null_bitmap` only
   // (its dictionary entry's bitmap does not contain it), so the null group below stays disjoint
   // from the value groups and no row is double-counted.
   for (const auto& [value_id, value_bitmap] : indexed_column.getIndexedValues()) {
      roaring::Roaring group = value_bitmap & filter_bitmap;
      if (group.isEmpty()) {
         continue;
      }
      result.emplace_back(std::string{indexed_column.lookupValue(value_id)}, std::move(group));
   }
   // The inverted index is an unordered map, so sort the value groups to give the node a
   // deterministic output order (the null group is appended last, mirroring the sequence path).
   std::ranges::sort(result, [](const auto& lhs, const auto& rhs) {
      return lhs.first < rhs.first;
   });
   roaring::Roaring null_group = indexed_column.null_bitmap & filter_bitmap;
   if (!null_group.isEmpty()) {
      result.emplace_back(std::nullopt, std::move(null_group));
   }
   return result;
}

schema::ColumnIdentifier IndexedColumnDimension::outputColumn() const {
   return {output_name, schema::ColumnType::STRING};
}

nlohmann::json IndexedColumnDimension::toJson() const {
   return {
      {"kind", "indexedColumn"},
      {"column", columnToJson(column)},
      {"outputName", output_name},
   };
}

BitmapAggregationNode::BitmapAggregationNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<scalar_expressions::ScalarExpression> filter,
   std::vector<std::unique_ptr<GroupingDimension>> dimensions,
   std::string count_field_name
)
    : table(std::move(table)),
      filter(std::move(filter)),
      dimensions(std::move(dimensions)),
      count_field_name(std::move(count_field_name)) {}

std::vector<schema::ColumnIdentifier> BitmapAggregationNode::getOutputSchema() const {
   std::vector<schema::ColumnIdentifier> output_fields;
   output_fields.reserve(dimensions.size() + 1);
   for (const auto& dimension : dimensions) {
      output_fields.emplace_back(dimension->outputColumn());
   }
   output_fields.emplace_back(count_field_name, schema::ColumnType::INT64);
   return output_fields;
}

nlohmann::json BitmapAggregationNode::toJson() const {
   nlohmann::json dimensions_json = nlohmann::json::array();
   for (const auto& dimension : dimensions) {
      dimensions_json.push_back(dimension->toJson());
   }
   return {
      {"type", nodeKindToString(kind())},
      {"filter", filter->toString()},
      {"dimensions", std::move(dimensions_json)},
      {"countFieldName", count_field_name},
   };
}

arrow::Result<arrow::acero::ExecNode*> BitmapAggregationNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& query_options
) const {
   auto filter_bitmap = computeFilter(filter, *table);
   const roaring::Roaring& filter_bitmap_ref = filter_bitmap.getConstReference();

   std::vector<GroupBitmaps> group_bitmaps_per_dimension;
   group_bitmaps_per_dimension.reserve(dimensions.size());
   for (const auto& dimension : dimensions) {
      group_bitmaps_per_dimension.push_back(dimension->buildGroups(*table, filter_bitmap_ref));
   }

   std::vector<GroupCombination> combinations =
      computeCombinations(filter_bitmap_ref, group_bitmaps_per_dimension);

   const size_t dimension_count = dimensions.size();

   // Materialize the combinations for `combinations[begin, end)` into a single ExecBatch.
   auto build_batch = [&](size_t begin, size_t end) -> arrow::Result<arrow::ExecBatch> {
      std::vector<arrow::StringBuilder> value_builders(dimension_count);
      arrow::Int64Builder count_builder;
      for (size_t combination_idx = begin; combination_idx < end; ++combination_idx) {
         const auto& combination = combinations[combination_idx];
         for (size_t i = 0; i < dimension_count; ++i) {
            const std::optional<std::string>& value =
               group_bitmaps_per_dimension[i][combination.group_indices[i]].first;
            if (value.has_value()) {
               ARROW_RETURN_NOT_OK(value_builders[i].Append(value.value()));
            } else {
               ARROW_RETURN_NOT_OK(value_builders[i].AppendNull());
            }
         }
         ARROW_RETURN_NOT_OK(count_builder.Append(static_cast<int64_t>(combination.count)));
      }

      std::vector<arrow::Datum> result_columns;
      result_columns.reserve(dimension_count + 1);
      for (auto& value_builder : value_builders) {
         arrow::Datum datum;
         ARROW_ASSIGN_OR_RAISE(datum, value_builder.Finish());
         result_columns.push_back(std::move(datum));
      }
      {
         arrow::Datum datum;
         ARROW_ASSIGN_OR_RAISE(datum, count_builder.Finish());
         result_columns.push_back(std::move(datum));
      }
      return arrow::ExecBatch::Make(result_columns);
   };

   // Emit the combinations in pipeline-sized batches instead of a single unbounded one: the number
   // of combinations is bounded only by the filtered row count, so a many-dimension query can
   // produce a very large result. `materialization_cutoff` is the batch-size-minus-one knob the
   // rest of the pipeline (e.g. the table scan) uses, so this output is sized the same way.
   const size_t batch_size = query_options.materialization_cutoff + 1;
   std::vector<arrow::ExecBatch> batches;
   for (size_t begin = 0; begin < combinations.size(); begin += batch_size) {
      const size_t end = std::min(begin + batch_size, combinations.size());
      ARROW_ASSIGN_OR_RAISE(arrow::ExecBatch batch, build_batch(begin, end));
      batches.push_back(std::move(batch));
   }

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [batches = std::move(batches),
       next_batch = size_t{0}]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (next_batch >= batches.size()) {
         const std::optional<arrow::ExecBatch> empty = std::nullopt;
         return arrow::Future{empty};
      }
      std::optional<arrow::ExecBatch> batch = std::move(batches[next_batch]);
      ++next_batch;
      return arrow::Future{std::move(batch)};
   };

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(getOutputSchema()),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", &plan, {}, options);
}

}  // namespace silo::query_engine::operators
