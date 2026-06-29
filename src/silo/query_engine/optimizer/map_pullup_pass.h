#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/optimizer/pipeline_pass_base.h"

namespace silo::query_engine::operators {
class FetchNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine::optimizer {

/// Optimization pass that moves a MapNode up the plan tree, so expensive per-row
/// computation (especially zstd decompression, which lives as a ScalarExpression inside
/// a MapNode) runs on as few rows as possible.
///
/// A MapNode is pulled up through a parent that reduces or reorders rows but does not
/// reference any column the MapNode produces:
///
/// ```
/// P(M(child))  →  M(P(child))
/// ```
///
/// Both the parent and the MapNode preserve row count and ordering, so the swap is
/// semantics-preserving. The main win is pulling a MapNode above a FetchNode (limit/offset)
/// so a `limit` no longer forces every row to be decompressed and then discarded.
///
/// Scope (this pass): pull through FetchNode only, which references no columns and is
/// therefore always safe. Pulling through FilterNode requires knowing the columns a filter
/// predicate references (Expression::freeIUs), which most predicate types do not yet report;
/// that, along with Project/OrderBy/Aggregate, is a follow-up (see #1336). The pass blocks
/// (leaves the MapNode in place) at every other node and at the root.
class MapPullupPass : public PipelinePassBase<MapPullupPass> {
  public:
   using PipelinePassBase<MapPullupPass>::operator();

   operators::QueryNodePtr operator()(operators::FetchNode& node);
};

}  // namespace silo::query_engine::optimizer
