#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/optimizer/pipeline_pass_base.h"

namespace silo::query_engine::operators {
class FetchNode;
class FilterNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine::optimizer {

/// Optimization pass that pulls MapNodes up through row-preserving operators when the parent
/// does not consume map-produced columns.
class MapPullupPass : public PipelinePassBase<MapPullupPass> {
  public:
   using PipelinePassBase<MapPullupPass>::operator();

   operators::QueryNodePtr operator()(operators::FetchNode& node);
   operators::QueryNodePtr operator()(operators::FilterNode& node);
};

}  // namespace silo::query_engine::optimizer
