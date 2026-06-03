#pragma once

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/count_filter_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine::operators {

template <typename Func>
// NOLINTNEXTLINE(misc-no-recursion)
decltype(auto) visit(QueryNode& node, Func&& func) {
   switch (node.kind()) {
      case NodeKind::AGGREGATE:
         return std::forward<Func>(func)(static_cast<AggregateNode&>(node));
      case NodeKind::PROJECT:
         return std::forward<Func>(func)(static_cast<ProjectNode&>(node));
      case NodeKind::MAP:
         return std::forward<Func>(func)(static_cast<MapNode&>(node));
      case NodeKind::ORDER_BY:
         return std::forward<Func>(func)(static_cast<OrderByNode&>(node));
      case NodeKind::FETCH:
         return std::forward<Func>(func)(static_cast<FetchNode&>(node));
      case NodeKind::FILTER:
         return std::forward<Func>(func)(static_cast<FilterNode&>(node));
      case NodeKind::UNRESOLVED_MUTATIONS_NUCLEOTIDE:
         return std::forward<Func>(func)(
            static_cast<UnresolvedMutationsNode<silo::Nucleotide>&>(node)
         );
      case NodeKind::UNRESOLVED_MUTATIONS_AMINO_ACID:
         return std::forward<Func>(func)(static_cast<UnresolvedMutationsNode<silo::AminoAcid>&>(node
         ));
      case NodeKind::UNRESOLVED_INSERTIONS_NUCLEOTIDE:
         return std::forward<Func>(func)(
            static_cast<UnresolvedInsertionsNode<silo::Nucleotide>&>(node)
         );
      case NodeKind::UNRESOLVED_INSERTIONS_AMINO_ACID:
         return std::forward<Func>(func)(
            static_cast<UnresolvedInsertionsNode<silo::AminoAcid>&>(node)
         );
      case NodeKind::UNRESOLVED_MOST_RECENT_COMMON_ANCESTOR:
         return std::forward<Func>(func)(static_cast<UnresolvedMostRecentCommonAncestorNode&>(node)
         );
      case NodeKind::UNRESOLVED_PHYLO_SUBTREE:
         return std::forward<Func>(func)(static_cast<UnresolvedPhyloSubtreeNode&>(node));
      case NodeKind::MUTATIONS_NUCLEOTIDE:
         return std::forward<Func>(func)(static_cast<MutationsNode<silo::Nucleotide>&>(node));
      case NodeKind::MUTATIONS_AMINO_ACID:
         return std::forward<Func>(func)(static_cast<MutationsNode<silo::AminoAcid>&>(node));
      case NodeKind::INSERTIONS_NUCLEOTIDE:
         return std::forward<Func>(func)(static_cast<InsertionsNode<silo::Nucleotide>&>(node));
      case NodeKind::INSERTIONS_AMINO_ACID:
         return std::forward<Func>(func)(static_cast<InsertionsNode<silo::AminoAcid>&>(node));
      case NodeKind::MOST_RECENT_COMMON_ANCESTOR:
         return std::forward<Func>(func)(static_cast<MostRecentCommonAncestorNode&>(node));
      case NodeKind::PHYLO_SUBTREE:
         return std::forward<Func>(func)(static_cast<PhyloSubtreeNode&>(node));
      case NodeKind::TABLE_SCAN:
         return std::forward<Func>(func)(static_cast<TableScanNode&>(node));
      case NodeKind::COUNT_FILTER:
         return std::forward<Func>(func)(static_cast<CountFilterNode&>(node));
      case NodeKind::ZSTD_DECOMPRESS:
         return std::forward<Func>(func)(static_cast<ZstdDecompressNode&>(node));
   }
   SILO_UNREACHABLE();
}

}  // namespace silo::query_engine::operators
