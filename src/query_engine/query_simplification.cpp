//
// Created by Alexander Taepper on 09.01.23.
//

#include <syncstream>
#include <silo/query_engine/query_engine.h>

using namespace silo;

std::unique_ptr<BoolExpression> PangoLineageEx::simplify(const Database& /*db*/, const DatabasePartition& dbp) const {
   if (lineageKey == UINT32_MAX) {
      return std::make_unique<EmptyEx>();
   }
   if (!this->includeSubLineages && !std::binary_search(dbp.sorted_lineages.begin(), dbp.sorted_lineages.end(), lineageKey)) {
      return std::make_unique<EmptyEx>();
   } else {
      return std::make_unique<PangoLineageEx>(lineageKey, includeSubLineages);
   }
}

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
         std::transform(and_child->negated_children.begin(), and_child->negated_children.end(),
                        std::back_inserter(ret->negated_children), [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); });
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

std::unique_ptr<BoolExpression> OrEx::simplify(const Database& db, const DatabasePartition& dbp) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(children.begin(), children.end(),
                  std::back_inserter(new_children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
   std::unique_ptr<OrEx> ret = std::make_unique<OrEx>();
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == EMPTY) {
         continue;
      } else if (child->type() == FULL) {
         return std::make_unique<FullEx>();
      } else if (child->type() == OR) {
         OrEx* or_child = dynamic_cast<OrEx*>(child.get());
         std::transform(or_child->children.begin(), or_child->children.end(),
                        std::back_inserter(new_children), [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); });
      } else {
         ret->children.push_back(std::move(child));
      }
   }
   if (ret->children.empty()) {
      return std::make_unique<EmptyEx>();
   }
   if (ret->children.size() == 1) {
      return std::move(ret->children[0]);
   }
   return ret;
}

std::unique_ptr<BoolExpression> NOfEx::simplify(const Database& db, const DatabasePartition& dbp) const {
   std::vector<std::unique_ptr<BoolExpression>> new_children;
   std::transform(children.begin(), children.end(),
                  std::back_inserter(new_children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
   std::unique_ptr<NOfEx> ret = std::make_unique<NOfEx>(n, impl, exactly);
   for (unsigned i = 0; i < new_children.size(); i++) {
      auto& child = new_children[i];
      if (child->type() == EMPTY) {
         continue;
      } else if (child->type() == FULL) {
         if (ret->n == 0) {
            if (ret->exactly) {
               return std::make_unique<EmptyEx>();
            } else {
               return std::make_unique<FullEx>();
            }
         }
         ret->n--;
      } else {
         ret->children.push_back(std::move(child));
      }
   }
   if (ret->n > ret->children.size()) {
      return std::make_unique<EmptyEx>();
   }
   if (ret->n == ret->children.size()) {
      auto new_ret = std::make_unique<AndEx>();
      for (auto& child : ret->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret;
   }
   if (ret->n == 0) {
      if (ret->exactly) {
         auto new_ret = std::make_unique<OrEx>();
         for (auto& child : ret->children) {
            new_ret->children.emplace_back(std::move(child));
         }
         return std::make_unique<NegEx>(std::move(new_ret));
      } else {
         return std::make_unique<EmptyEx>();
      }
   }
   if (ret->n == 1 && !ret->exactly) {
      auto new_ret = std::make_unique<OrEx>();
      for (auto& child : ret->children) {
         new_ret->children.emplace_back(std::move(child));
      }
      return new_ret;
   }
   if (ret->children.empty()) {
      return std::make_unique<EmptyEx>();
   }
   if (ret->children.size() == 1) {
      return std::move(ret->children[0]);
   }
   return ret;
}
