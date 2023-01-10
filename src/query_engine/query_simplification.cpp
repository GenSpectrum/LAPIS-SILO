//
// Created by Alexander Taepper on 09.01.23.
//

#include <silo/query_engine/query_engine.h>

using namespace silo;

std::unique_ptr<BoolExpression> AndEx::simplify(const Database& db, const DatabasePartition& dbp) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(children.begin(), children.end(),
                  std::back_inserter(new_children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
   std::unique_ptr<AndEx> ret = std::make_unique<AndEx>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == FULL) {
         continue;
      } else if (child->type() == EMPTY) {
         return std::make_unique<EmptyEx>();
      } else if (child->type() == AND) {
         AndEx* and_child = dynamic_cast<AndEx*>(child.get());
         std::transform(and_child->children.begin(), and_child->children.end(),
                        std::back_inserter(new_children), [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); });
      } else if (child->type() == NEG) {
         NegEx* negated_child = dynamic_cast<NegEx*>(child.get());
         ret->negated_children.emplace_back(std::move(negated_child->child));
      } else {
         ret->children.push_back(std::move(child));
      }
   }
   if (ret->children.empty() && ret->negated_children.empty()) {
      return std::make_unique<FullEx>();
   }
   if (ret->children.size() == 1 && ret->negated_children.empty()) {
      return std::move(ret->children[0]);
   }
   return ret;
}
