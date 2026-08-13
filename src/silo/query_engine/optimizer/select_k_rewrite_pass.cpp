#include "silo/query_engine/optimizer/select_k_rewrite_pass.h"

#include <memory>
#include <utility>

#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/order_by_with_limit_node.h"
#include "silo/query_engine/operators/query_node.h"

namespace rhydb::query_engine::optimizer {

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr SelectKRewritePass::operator()(operators::FetchNode& node) {
   propagateToNode(node.child);

   // select_k needs a bounded number of rows to keep; an offset-only fetch has none.
   if (!node.count.has_value()) {
      return nullptr;
   }
   if (node.child->kind() != operators::NodeKind::ORDER_BY) {
      return nullptr;
   }
   auto& order_by = static_cast<operators::OrderByNode&>(*node.child);

   // An order-by with neither sort fields nor a randomize seed performs no ordering at all, so
   // there is nothing to combine. A randomize seed (with or without fields) is fine: the combined
   // node applies the same random-hash top-k.
   if (order_by.fields.empty() && !order_by.randomize_seed.has_value()) {
      return nullptr;
   }

   return std::make_unique<operators::OrderByWithLimitNode>(
      std::move(order_by.child),
      std::move(order_by.fields),
      node.count.value(),
      node.offset,
      order_by.randomize_seed
   );
}

}  // namespace rhydb::query_engine::optimizer
