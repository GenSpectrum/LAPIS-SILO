#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/optimizer/pipeline_pass_base.h"

namespace rhydb::query_engine::operators {
class FetchNode;
}  // namespace rhydb::query_engine::operators

namespace rhydb::query_engine::optimizer {

/// Optimization pass that combines a `FetchNode` with a finite limit sitting directly above an
/// `OrderByNode` into a single `OrderByWithLimitNode`:
///
/// ```
/// Fetch(count, offset)(OrderBy(fields))  →  OrderByWithLimit(fields, count, offset)
/// ```
///
/// The combined node uses Arrow's `select_k` (top-k) implementation, which keeps only the
/// `offset + count` smallest rows instead of fully sorting the input and then discarding all but a
/// small window.
class SelectKRewritePass : public PipelinePassBase<SelectKRewritePass> {
  public:
   using PipelinePassBase<SelectKRewritePass>::operator();

   operators::QueryNodePtr operator()(operators::FetchNode& node);
};

}  // namespace rhydb::query_engine::optimizer
