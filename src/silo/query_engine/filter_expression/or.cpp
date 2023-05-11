#include "silo/query_engine/filter_expressions/or.h"

#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/union.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

Or::Or(std::vector<std::unique_ptr<Expression>>&& children)
    : children(std::move(children)) {}

std::string Or::toString(const silo::Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += " | ";
      res += child->toString(database);
   }
   res += ")";
   return res;
}

std::unique_ptr<operators::Operator> Or::compile(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<operators::Operator>> all_child_operators;
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(all_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(database, database_partition);
      }
   );
   std::vector<std::unique_ptr<operators::Operator>> filtered_child_operators;
   for (auto& child : all_child_operators) {
      if (child->type() == operators::EMPTY) {
         continue;
      }
      if (child->type() == operators::FULL) {
         return std::make_unique<operators::Full>(database_partition.sequenceCount);
      }
      if (child->type() == operators::UNION) {
         auto* or_child = dynamic_cast<operators::Union*>(child.get());
         std::transform(
            or_child->children.begin(),
            or_child->children.end(),
            std::back_inserter(filtered_child_operators),
            [&](std::unique_ptr<operators::Operator>& expression) { return std::move(expression); }
         );
      }
      filtered_child_operators.push_back(std::move(child));
   }
   if (filtered_child_operators.empty()) {
      return std::make_unique<operators::Empty>();
   }
   if (filtered_child_operators.size() == 1) {
      return std::move(filtered_child_operators[0]);
   }

   if (std::any_of(
          filtered_child_operators.begin(),
          filtered_child_operators.end(),
          [](const auto& child) { return child->type() == operators::COMPLEMENT; }
       )) {
      /// Eliminate negation by using De'Morgan's rule and turning union into intersection
      std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
      std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
      for (auto& child : filtered_child_operators) {
         if (child->type() == operators::COMPLEMENT) {
            negated_child_operators.emplace_back(
               std::move(dynamic_cast<operators::Complement*>(child.get())->child)
            );
         } else {
            non_negated_child_operators.push_back(std::move(child));
         }
      }
      /// Now swap negated children and non-negated ones
      auto intersection = std::make_unique<operators::Intersection>(
         std::move(negated_child_operators), std::move(non_negated_child_operators)
      );
      return std::make_unique<operators::Complement>(
         std::move(intersection), database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::Union>(std::move(filtered_child_operators));
}

}  // namespace silo::query_engine::filter_expressions
