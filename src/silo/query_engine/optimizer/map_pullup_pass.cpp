#include "silo/query_engine/optimizer/map_pullup_pass.h"

#include <memory>

#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/map_node.h"

namespace silo::query_engine::optimizer {

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr MapPullupPass::operator()(operators::FetchNode& node) {
   propagateToNode(node.child);

   // A FetchNode only limits/offsets rows; it references no columns, so a MapNode below
   // it can always be pulled up.
   if (node.child->kind() != operators::NodeKind::MAP) {
      return nullptr;
   }
   operators::QueryNodePtr map_owner = std::move(node.child);
   auto& map = static_cast<operators::MapNode&>(*map_owner);

   auto new_fetch =
      std::make_unique<operators::FetchNode>(std::move(map.child), node.count, node.offset);
   map.child = std::move(new_fetch);
   return map_owner;
}

}  // namespace silo::query_engine::optimizer
