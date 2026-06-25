#pragma once

#include <vector>

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/pipeline_pass_base.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {
class TableScanNode;
class AggregateNode;
class ProjectNode;
class MapNode;
class ZstdDecompressNode;
class OrderByNode;
class UnionAllNode;
}  // namespace silo::query_engine::operators

namespace silo::query_engine {

/// Optimization pass that narrows the set of columns produced by each node down to those
/// actually required by its parent, pruning unneeded columns from scans and collapsing
/// redundant Project/ZstdDecompress nodes.
///
/// Carries state (`required`) that is mutated while walking the nodes.
class ColumnNarrowingPass : public PipelinePassBase<ColumnNarrowingPass> {
  public:
   using RequiredColumns = std::vector<schema::ColumnIdentifier>;

   RequiredColumns required;

   explicit ColumnNarrowingPass(RequiredColumns required)
       : required(std::move(required)) {}

   static ColumnNarrowingPass makePass(const operators::QueryNodePtr& node);

   using PipelinePassBase<ColumnNarrowingPass>::operator();

   operators::QueryNodePtr operator()(operators::TableScanNode& node);
   operators::QueryNodePtr operator()(operators::AggregateNode& node);
   operators::QueryNodePtr operator()(operators::ProjectNode& node);
   operators::QueryNodePtr operator()(operators::ZstdDecompressNode& node);
   operators::QueryNodePtr operator()(operators::MapNode& node);
   operators::QueryNodePtr operator()(operators::OrderByNode& node);
   operators::QueryNodePtr operator()(operators::UnionAllNode& node);
};

}  // namespace silo::query_engine
