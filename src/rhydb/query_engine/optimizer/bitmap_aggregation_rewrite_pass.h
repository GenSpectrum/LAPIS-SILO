#pragma once

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/optimizer/pipeline_pass_base.h"

namespace rhydb::query_engine::operators {
class AggregateNode;
}  // namespace rhydb::query_engine::operators

namespace rhydb::query_engine::optimizer {

/// Optimization pass that recognizes a `groupBy` with a single `count()` whose grouping keys can be
/// computed directly from roaring bitmaps, and turns it into the dedicated, far cheaper
/// BitmapAggregationNode pipeline.
///
/// A grouping key is either computed by a directly preceding `map`, or -- when no assignment
/// produces it -- read straight from the table scan as the same-named column; both are resolved the
/// same way, so a plain table field and the equivalent `map({x := field})` assignment behave
/// identically. Each key must then be one of:
///   * a sequence-position lookup (`At` over a sequence column, the mutation co-occurrence
///     pattern), e.g.
///
///         ... | map({symbol_123 := main.at(123)}) | groupBy({count := count()}, {symbol_123})
///
///   * a bare read of a string-valued column: an indexed (dictionary-encoded) one is grouped
///     straight from its inverted index, a plain one by scanning it to build the per-value bitmaps,
///     e.g.
///
///         ... | groupBy({count := count()}, {division, host})
///
///   * any other scalar expression the grouper can evaluate through Arrow over the columns it
///     reads -- including a plain non-string field such as a date, int or bool -- which is grouped
///     by the (typed) value it produces, e.g.
///
///         ... | map({week := date.isoWeek()}) | groupBy({count := count()}, {week, date})
///
/// These may be mixed freely within one `groupBy`. In query-node terms this is an `AggregateNode`
/// whose only aggregate is `count()` and all of whose grouping keys resolve against the leaf table
/// scan. Such a node is replaced by a `BitmapAggregationNode`, which computes the grouping directly
/// from the per-value roaring bitmaps instead of materializing one row per sequence and hashing it.
/// Queries that don't match this shape are left untouched, so the generic map/groupBy execution
/// still handles every other case.
///
/// This pass runs after FilterPushdownPass so the matched pipeline's leaf has already been
/// collapsed into a single `TableScanNode` carrying the full filter, which the rewrite reads to
/// resolve each grouping key against the table schema. Traversal into every other node is provided
/// by PipelinePassBase; only `AggregateNode` needs custom handling.
class BitmapAggregationRewritePass : public PipelinePassBase<BitmapAggregationRewritePass> {
  public:
   using PipelinePassBase<BitmapAggregationRewritePass>::operator();

   operators::QueryNodePtr operator()(operators::AggregateNode& node);
};

}  // namespace rhydb::query_engine::optimizer
