#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/optimizer/pipeline_pass_base.h"

namespace silo::query_engine::operators {
class AggregateNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine::optimizer {

/// Optimization pass that recognizes a `groupBy` with a single `count()` whose grouping keys can be
/// computed directly from roaring bitmaps, and turns it into the dedicated, far cheaper
/// BitmapAggregationNode pipeline.
///
/// Each grouping key must be one of:
///   * a sequence-position lookup produced by an `At` assignment of a directly preceding `map`
///     (the mutation co-occurrence pattern), e.g.
///
///         ... | map({symbol_123 := main.at(123)}) | groupBy({count := count()}, {symbol_123})
///
///   * an indexed string column read straight from the table scan, e.g.
///
///         ... | groupBy({count := count()}, {division})
///
/// The two may be mixed within one `groupBy`. In query-node terms this is an `AggregateNode` whose
/// only aggregate is `count()` and all of whose grouping keys resolve, against the leaf table scan,
/// to either an `At`-derived sequence position or an indexed string column. Such a node is replaced
/// by a `BitmapAggregationNode`, which computes the grouping directly from the per-value roaring
/// bitmaps instead of materializing one row per sequence and hashing it. Queries that don't match
/// this shape are left untouched, so the generic map/groupBy execution still handles every other
/// case.
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

}  // namespace silo::query_engine::optimizer
