#include "rhydb/query_engine/operators/bitmap_aggregation_node.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/common/panic.h"
#include "rhydb/common/symbol_map.h"
#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/operators/compute_filter.h"
#include "rhydb/roaring_util/roaring_container.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/column/dictionary_encoded_column.h"
#include "rhydb/storage/column/sequence_column.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

namespace {

using roaring_util::RoaringContainer;
using roaring_util::RoaringContainerView;

/// One observed combination across the grouping dimensions together with the number of (filtered)
/// rows carrying it. Each entry of `group_indices` indexes into the corresponding dimension's
/// group labels, i.e. it identifies which group was chosen in that dimension. Keeping indices
/// rather than the values themselves makes the recursion type-agnostic: the actual value is
/// resolved per dimension only when the output is materialized.
struct GroupCombination {
   std::vector<size_t> group_indices;
   uint64_t count;
};

/// Per dimension, the value each group index stands for: `labels[group_index]` is the string a
/// group renders to in the output, or `std::nullopt` for the null group (rendered as a SQL null).
/// A dimension keeps a fixed label for every group it could ever emit, even ones that turn out
/// empty; the recursion only ever references the indices of combinations that actually occur.
using GroupLabels = std::vector<std::optional<std::string>>;

/// The groups of one dimension that have a container in a given 2^16 chunk, as (group index,
/// non-owning container view) pairs. The views are NOT yet intersected with the query filter -- the
/// aggregation seeds each chunk's recursion with the filter container and intersects it in, so the
/// views can be handed out straight from the underlying index storage with no copy.
using DimensionGroupsInChunk = std::vector<std::pair<size_t, RoaringContainerView>>;

/// A grouping dimension resolved against the table, producing its groups one 2^16 chunk at a time.
///
/// The whole point of this interface is that grouping never materializes whole-table per-group
/// bitmaps: the aggregation drives it chunk by chunk, and each chunk's groups are handed back as
/// `RoaringContainerView`s that (wherever possible) point straight into the column's stored
/// containers. `labels` maps the stable group indices these views carry to their output values.
class ChunkGrouper {
  public:
   /// `group_index` -> the value that group renders to (see `GroupLabels`).
   GroupLabels labels;

   ChunkGrouper() = default;
   ChunkGrouper(const ChunkGrouper&) = delete;
   ChunkGrouper& operator=(const ChunkGrouper&) = delete;
   ChunkGrouper(ChunkGrouper&&) = delete;
   ChunkGrouper& operator=(ChunkGrouper&&) = delete;
   virtual ~ChunkGrouper() = default;

   /// This dimension's groups within the 2^16 chunk `chunk_key`, as (group index, view) pairs. The
   /// views must stay valid for the caller's aggregation of this chunk: those that point into the
   /// column's own storage are always valid, while groups that have to be *computed* (a sequence
   /// position's reference / missing groups) are appended to `scratch` as owning containers and
   /// viewed from there, so `scratch` must outlive the returned views. `filter_view` is the query
   /// filter's container for this chunk; it is passed as a bare container view rather than a bitmap
   /// so a grouper that does not need it (an indexed column) pays nothing, and one that does (a
   /// sequence position, whose "not covered at this position" group is inherently filter-relative)
   /// uses it directly.
   [[nodiscard]] virtual DimensionGroupsInChunk groupChunk(
      uint16_t chunk_key,
      RoaringContainerView filter_view,
      std::vector<RoaringContainer>& scratch
   ) const = 0;
};

/// Groups the rows by the symbol they carry at a fixed sequence position, straight from the
/// column's vertical mutation index and horizontal coverage index. Plain mutation groups are handed
/// out as zero-copy views into the vertical index's stored containers; only the reference, missing
/// and null groups -- which are defined by set arithmetic against the coverage and the filter --
/// are computed per chunk. The reference / missing (no coverage) / null handling mirrors
/// `SymbolInSet` exactly so the grouping matches the generic `at()`/groupBy path:
///   * a plain mutation symbol -> the rows carrying that mutation at the position,
///   * the local reference symbol -> the covered rows carrying no other mutation,
///   * the missing symbol -> the not-covered rows (plus any explicit missing mutation), minus
///   nulls,
///   * a null sequence -> its own null group (it carries no symbol at any position).
template <typename SymbolType>
class SequencePositionGrouper : public ChunkGrouper {
   static constexpr size_t SYMBOL_COUNT = SymbolType::SYMBOLS.size();
   // The null group's index sits just past every symbol rank, so the group-index tuples sort it
   // last -- the same output order as the generic path.
   static constexpr size_t NULL_INDEX = SYMBOL_COUNT;

   const storage::column::SequenceColumn<SymbolType>& column;
   uint32_t position_idx;
   typename SymbolType::Symbol reference_symbol;
   typename SymbolType::Symbol missing_symbol = SymbolType::SYMBOL_MISSING;

   // group index (== SYMBOLS rank) of each symbol, so a stored mutation container maps to the label
   // (and output order) it belongs to regardless of the enum's underlying values.
   SymbolMap<SymbolType, size_t> rank_of_symbol;

   // The position's vertical mutation containers, bucketed by 2^16 chunk so the per-chunk work is a
   // lookup rather than a rescan of the position's whole diff range. Views into the stored index.
   std::map<uint16_t, std::vector<std::pair<typename SymbolType::Symbol, RoaringContainerView>>>
      mutations_by_chunk;
   // The column's null rows, one view per 2^16 chunk (into `column.null_bitmap`).
   std::map<uint16_t, RoaringContainerView> null_views;

  public:
   SequencePositionGrouper(
      const storage::column::SequenceColumn<SymbolType>& column,
      uint32_t position_idx
   )
       : column(column),
         position_idx(position_idx) {
      CHECK_SILO_QUERY(
         position_idx < column.metadata->reference_sequence.size(),
         "SymbolInSet<{}> position is out of bounds {} > {}",
         SymbolType::SYMBOL_NAME,
         position_idx + 1,
         column.metadata->reference_sequence.size()
      );
      reference_symbol = column.getLocalReferencePosition(position_idx);

      labels.reserve(SYMBOL_COUNT + 1);
      for (size_t rank = 0; rank < SYMBOL_COUNT; ++rank) {
         const typename SymbolType::Symbol symbol = SymbolType::SYMBOLS.at(rank);
         rank_of_symbol[symbol] = rank;
         labels.emplace_back(std::string(1, SymbolType::symbolToChar(symbol)));
      }
      labels.emplace_back(std::nullopt);  // NULL_INDEX

      auto [diff_it, diff_end] = column.vertical_sequence_index.getRangeForPosition(position_idx);
      for (; diff_it != diff_end; ++diff_it) {
         mutations_by_chunk[diff_it->first.v_index].emplace_back(
            diff_it->first.symbol, RoaringContainerView(diff_it->second)
         );
      }

      for (auto& [chunk_key, view] : CopyOnWriteBitmap{&column.null_bitmap}.containerViews()) {
         null_views.emplace(chunk_key, view);
      }
   }

   [[nodiscard]] DimensionGroupsInChunk groupChunk(
      uint16_t chunk_key,
      RoaringContainerView filter_view,
      std::vector<RoaringContainer>& scratch
   ) const override {
      DimensionGroupsInChunk groups;
      // At most one computed group per symbol (reference / missing); reserve so the views taken
      // below never dangle across a reallocation.
      scratch.reserve(SYMBOL_COUNT);

      // The chunk's per-symbol mutation containers (raw views), plus the unions the reference /
      // both cases subtract. The reference symbol is never itself a stored mutation, so excluding
      // it is a no-op in practice, but it is kept explicit to mirror the generic path exactly.
      std::array<std::optional<RoaringContainerView>, SYMBOL_COUNT> mutation_view;
      RoaringContainer mutations_except_missing;
      RoaringContainer mutations_except_reference_and_missing;
      if (auto chunk_mutations = mutations_by_chunk.find(chunk_key);
          chunk_mutations != mutations_by_chunk.end()) {
         for (const auto& [symbol, view] : chunk_mutations->second) {
            mutation_view[rank_of_symbol.at(symbol)] = view;
            if (symbol != missing_symbol) {
               mutations_except_missing |= view;
               if (symbol != reference_symbol) {
                  mutations_except_reference_and_missing |= view;
               }
            }
         }
      }

      // The covered rows come back as a single chunk container; an empty container means the
      // position is not covered anywhere in the chunk.
      const RoaringContainer covered =
         column.horizontal_coverage_index.coveredRowsInChunk(position_idx, chunk_key);

      // A null sequence carries no symbol at any position, so it forms its own group and is
      // excluded from the missing symbol. The group itself is a zero-copy view; `null_in_chunk`
      // (filter-bound) is only what the missing computation subtracts.
      std::optional<RoaringContainerView> null_view;
      RoaringContainer null_in_chunk;
      if (auto null_iter = null_views.find(chunk_key); null_iter != null_views.end()) {
         null_view = null_iter->second;
         null_in_chunk = filter_view & *null_view;
      }

      // Appends a computed (owning) group to the result, keeping its container alive in `scratch`.
      const auto pushComputed = [&](size_t group_index, RoaringContainer&& group) {
         if (group.empty()) {
            return;
         }
         scratch.push_back(std::move(group));
         groups.emplace_back(group_index, RoaringContainerView{scratch.back()});
      };

      for (size_t rank = 0; rank < SYMBOL_COUNT; ++rank) {
         const typename SymbolType::Symbol symbol = SymbolType::SYMBOLS.at(rank);
         const bool is_reference = symbol == reference_symbol;
         const bool is_missing = symbol == missing_symbol;

         if (!is_reference && !is_missing) {
            // Plain mutation: hand the stored container straight through, no copy and no filtering
            // (the aggregation intersects the filter in).
            if (mutation_view[rank].has_value()) {
               groups.emplace_back(rank, *mutation_view[rank]);
            }
            continue;
         }

         RoaringContainer group;
         if (is_reference && is_missing) {
            // The local reference symbol is itself the missing symbol: every filtered row without
            // an explicit other mutation carries it, minus the sequence-less rows.
            group = filter_view - RoaringContainerView{mutations_except_missing};
            group = RoaringContainerView{group} - RoaringContainerView{null_in_chunk};
         } else if (is_missing) {
            group = filter_view - RoaringContainerView{covered};  // not covered here ...
            if (mutation_view[rank].has_value()) {
               // ... plus any explicit missing mutation (bounded by the filter) ...
               const RoaringContainer explicit_missing = filter_view & *mutation_view[rank];
               group = RoaringContainerView{group} | RoaringContainerView{explicit_missing};
            }
            // ... but a null sequence carries no symbol.
            group = RoaringContainerView{group} - RoaringContainerView{null_in_chunk};
         } else {  // is_reference
            // covered ...
            const RoaringContainer covered_in_filter = filter_view & RoaringContainerView{covered};
            // ... and carrying no other mutation.
            group = RoaringContainerView{covered_in_filter} -
                    RoaringContainerView{mutations_except_reference_and_missing};
         }
         pushComputed(rank, std::move(group));
      }

      if (null_view.has_value()) {
         groups.emplace_back(NULL_INDEX, *null_view);
      }
      return groups;
   }
};

/// Groups the rows by the value of an indexed string column, straight from its inverted index. Each
/// value's rows are handed out as zero-copy views into the stored per-value bitmap (one per 2^16
/// chunk), plus a null group from the column's null bitmap. Value groups get consecutive indices in
/// sorted-value order (null last), so the combinations come out in the same order as the generic
/// path.
class IndexedColumnGrouper : public ChunkGrouper {
   // chunk key -> the groups holding a container in that chunk. Precomputed once (the inverted
   // index is unordered), so per-chunk grouping is a single map lookup returning views into stored
   // bitmaps.
   std::map<uint16_t, DimensionGroupsInChunk> groups_by_chunk;

  public:
   explicit IndexedColumnGrouper(const storage::column::DictionaryEncodedColumn& column) {
      // One group per distinct value, ordered by the value string so the node has a deterministic
      // output order. A null row lives only in `null_bitmap` (its value's bitmap does not contain
      // it), so the null group stays disjoint from the value groups and no row is double-counted.
      std::vector<std::pair<std::string, const roaring::Roaring*>> values;
      values.reserve(column.getIndexedValues().size());
      for (const auto& [value_id, value_bitmap] : column.getIndexedValues()) {
         values.emplace_back(std::string{column.lookupValue(value_id)}, &value_bitmap);
      }
      std::ranges::sort(values, [](const auto& lhs, const auto& rhs) {
         return lhs.first < rhs.first;
      });

      labels.reserve(values.size() + 1);
      for (size_t group_index = 0; group_index < values.size(); ++group_index) {
         for (auto& [chunk_key, view] :
              CopyOnWriteBitmap{values[group_index].second}.containerViews()) {
            groups_by_chunk[chunk_key].emplace_back(group_index, view);
         }
         labels.emplace_back(std::move(values[group_index].first));
      }
      const size_t null_index = values.size();
      labels.emplace_back(std::nullopt);  // null_index
      for (auto& [chunk_key, view] : CopyOnWriteBitmap{&column.null_bitmap}.containerViews()) {
         groups_by_chunk[chunk_key].emplace_back(null_index, view);
      }
   }

   [[nodiscard]] DimensionGroupsInChunk groupChunk(
      uint16_t chunk_key,
      RoaringContainerView /*filter_view*/,
      std::vector<RoaringContainer>& /*scratch*/
   ) const override {
      if (auto iter = groups_by_chunk.find(chunk_key); iter != groups_by_chunk.end()) {
         return iter->second;
      }
      return {};
   }
};

std::unique_ptr<ChunkGrouper> makeGrouper(
   const SequencePositionDimension& dimension,
   const storage::Table& table
) {
   if (dimension.is_nucleotide) {
      const auto& column = table.columns.getColumns<Nucleotide::Column>().at(dimension.column.name);
      return std::make_unique<SequencePositionGrouper<Nucleotide>>(column, dimension.position_idx);
   }
   const auto& column = table.columns.getColumns<AminoAcid::Column>().at(dimension.column.name);
   return std::make_unique<SequencePositionGrouper<AminoAcid>>(column, dimension.position_idx);
}

std::unique_ptr<ChunkGrouper> makeGrouper(
   const IndexedColumnDimension& dimension,
   const storage::Table& table
) {
   const auto& column =
      table.columns.getColumns<storage::column::DictionaryEncodedColumn>().at(dimension.column.name
      );
   return std::make_unique<IndexedColumnGrouper>(column);
}

/// Recursively intersect one chunk's per-dimension group containers, depth by depth, and add the
/// cardinality of each surviving full combination to `counts`. `current` is the running
/// container-level intersection of the groups chosen so far; it is seeded with the filter chunk (so
/// the not-yet-filtered group views are bounded by the filter here) and is only ever a view or a
/// short-lived owned temporary, so no `CopyOnWriteBitmap` is built and no group container is
/// copied. Empty intersections are pruned.
// NOLINTNEXTLINE(misc-no-recursion)
void aggregateChunk(
   size_t depth,
   const roaring::internal::container_t* current,
   uint8_t current_typecode,
   const std::vector<const DimensionGroupsInChunk*>& groups_by_dimension,
   std::vector<size_t>& chosen_indices,
   std::map<std::vector<size_t>, uint64_t>& counts
) {
   const size_t last_dimension = groups_by_dimension.size() - 1;
   for (const auto& [group_index, view] : *groups_by_dimension[depth]) {
      chosen_indices[depth] = group_index;

      if (depth == last_dimension) {
         // Leaf: only the cardinality of the final intersection is needed, so compute it directly
         // without allocating a result container.
         const uint64_t count = static_cast<uint64_t>(roaring::internal::container_and_cardinality(
            current, current_typecode, view.rawContainer(), view.getTypecode()
         ));
         if (count > 0) {
            counts[chosen_indices] += count;
         }
         continue;
      }

      uint8_t result_typecode = 0;
      auto* intersection = roaring::internal::container_and(
         current, current_typecode, view.rawContainer(), view.getTypecode(), &result_typecode
      );
      if (roaring::internal::container_nonzero_cardinality(intersection, result_typecode)) {
         aggregateChunk(
            depth + 1, intersection, result_typecode, groups_by_dimension, chosen_indices, counts
         );
      }
      roaring::internal::container_free(intersection, result_typecode);
   }
}

/// Computes the co-occurrence counts one 2^16 chunk at a time. It enumerates the filter's
/// containers and, for each (necessarily non-empty) chunk, builds every dimension's groups for just
/// that chunk
/// -- handing out `RoaringContainerView`s into the columns' stored containers wherever possible --
/// then intersects them recursively against the filter chunk. A combination spans chunks, so the
/// surviving cardinalities are summed across them. Only non-empty combinations are visited, so this
/// scales with the number of matching rows rather than the Cartesian product of the dimensions.
std::vector<GroupCombination> computeCombinations(
   const std::vector<std::unique_ptr<ChunkGrouper>>& groupers,
   const CopyOnWriteBitmap& filter_bitmap
) {
   const size_t num_dimensions = groupers.size();
   if (num_dimensions == 0) {
      return {};
   }

   // Counts keyed by the group-index tuple; std::map keeps the output in ascending
   // (lexicographic-by-index) order, which -- because every dimension numbers its groups in output
   // order -- is exactly the order the result rows should appear in.
   std::map<std::vector<size_t>, uint64_t> counts;
   std::vector<size_t> chosen_indices(num_dimensions);

   for (const auto& [chunk_key, filter_view] : filter_bitmap.containerViews()) {
      // Build each dimension's groups for this chunk. `scratch` backs the computed (non-view) group
      // containers and must outlive the aggregation below, so it lives for the whole chunk
      // iteration.
      std::vector<std::vector<RoaringContainer>> scratch(num_dimensions);
      std::vector<DimensionGroupsInChunk> groups(num_dimensions);
      bool any_dimension_empty = false;
      for (size_t dimension = 0; dimension < num_dimensions; ++dimension) {
         groups[dimension] =
            groupers[dimension]->groupChunk(chunk_key, filter_view, scratch[dimension]);
         if (groups[dimension].empty()) {
            // No group covers this chunk in this dimension -> no combination can survive it.
            any_dimension_empty = true;
            break;
         }
      }
      if (any_dimension_empty) {
         continue;
      }

      std::vector<const DimensionGroupsInChunk*> groups_at_chunk(num_dimensions);
      for (size_t dimension = 0; dimension < num_dimensions; ++dimension) {
         groups_at_chunk[dimension] = &groups[dimension];
      }
      aggregateChunk(
         0,
         filter_view.rawContainer(),
         filter_view.getTypecode(),
         groups_at_chunk,
         chosen_indices,
         counts
      );
   }

   std::vector<GroupCombination> combinations;
   combinations.reserve(counts.size());
   for (const auto& [indices, count] : counts) {
      combinations.push_back(GroupCombination{.group_indices = indices, .count = count});
   }
   return combinations;
}

/// Materializes the combinations for `combinations[begin, end)` into a single ExecBatch: one string
/// column per dimension (holding that dimension's group value, or null) plus the int64 count
/// column.
// The cognitive-complexity count comes entirely from the ARROW_RETURN_NOT_OK/ARROW_ASSIGN_OR_RAISE
// error-check macros, not from real branching; the logic is a straight-line append loop.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<arrow::ExecBatch> buildBatch(
   const std::vector<GroupCombination>& combinations,
   const std::vector<GroupLabels>& labels_per_dimension,
   size_t dimension_count,
   size_t begin,
   size_t end
) {
   std::vector<arrow::StringBuilder> value_builders(dimension_count);
   arrow::Int64Builder count_builder;
   for (size_t combination_idx = begin; combination_idx < end; ++combination_idx) {
      const auto& combination = combinations[combination_idx];
      for (size_t i = 0; i < dimension_count; ++i) {
         const std::optional<std::string>& value =
            labels_per_dimension[i][combination.group_indices[i]];
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

schema::ColumnIdentifier SequencePositionDimension::outputColumn() const {
   return {.name = output_name, .type = schema::ColumnType::STRING};
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

schema::ColumnIdentifier IndexedColumnDimension::outputColumn() const {
   return {.name = output_name, .type = schema::ColumnType::STRING};
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
   std::vector<GroupingDimension> dimensions,
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
      output_fields.emplace_back(
         std::visit([](const auto& dim) { return dim.outputColumn(); }, dimension)
      );
   }
   output_fields.emplace_back(count_field_name, schema::ColumnType::INT64);
   return output_fields;
}

nlohmann::json BitmapAggregationNode::toJson() const {
   nlohmann::json dimensions_json = nlohmann::json::array();
   for (const auto& dimension : dimensions) {
      dimensions_json.push_back(std::visit([](const auto& dim) { return dim.toJson(); }, dimension)
      );
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

   // Resolve each dimension against the table into a grouper that produces its groups per 2^16
   // chunk (this also validates, e.g. a sequence position out of range throws here). The groups are
   // then built and counted chunk by chunk, never materializing whole-table per-group bitmaps.
   std::vector<std::unique_ptr<ChunkGrouper>> groupers;
   groupers.reserve(dimensions.size());
   for (const auto& dimension : dimensions) {
      groupers.push_back(
         std::visit([&](const auto& dim) { return makeGrouper(dim, *table); }, dimension)
      );
   }

   std::vector<GroupCombination> combinations = computeCombinations(groupers, filter_bitmap);

   const size_t dimension_count = dimensions.size();

   // The group bitmaps are no longer needed once counting is done; keep only the per-dimension
   // value labels the output materialization resolves the group indices against.
   std::vector<GroupLabels> labels_per_dimension;
   labels_per_dimension.reserve(groupers.size());
   for (auto& grouper : groupers) {
      labels_per_dimension.push_back(std::move(grouper->labels));
   }

   // Emit the combinations in pipeline-sized batches instead of a single unbounded one, and build
   // each batch only when the downstream pulls it rather than materializing the whole result up
   // front: the number of combinations is bounded only by the filtered row count, so a
   // many-dimension query can produce a very large result and holding it all at once would blow up
   // peak memory. `materialization_cutoff` is the batch-size-minus-one knob the rest of the
   // pipeline (e.g. the table scan) uses, so this output is sized the same way.
   const size_t batch_size = query_options.materialization_cutoff + 1;

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [combinations = std::move(combinations),
       labels_per_dimension = std::move(labels_per_dimension),
       dimension_count,
       batch_size,
       begin = size_t{0}]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (begin >= combinations.size()) {
         return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(std::nullopt);
      }
      const size_t end = std::min(begin + batch_size, combinations.size());
      arrow::Result<arrow::ExecBatch> batch =
         buildBatch(combinations, labels_per_dimension, dimension_count, begin, end);
      begin = end;
      return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(batch.Map(
         [](arrow::ExecBatch value) { return std::optional<arrow::ExecBatch>{std::move(value)}; }
      ));
   };

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(getOutputSchema()),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", &plan, {}, options);
}

}  // namespace rhydb::query_engine::operators
