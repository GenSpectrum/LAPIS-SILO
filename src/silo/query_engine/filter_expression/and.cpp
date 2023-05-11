#include "silo/query_engine/filter_expressions/and.h"

#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/union.h"
#include "silo/storage/database_partition.h"

namespace operators = silo::query_engine::operators;

namespace silo::query_engine::filter_expressions {

And::And(std::vector<std::unique_ptr<Expression>>&& children)
    : children(std::move(children)) {}

std::string And::toString(const silo::Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += " & ";
      res += child->toString(database);
   }
   res += ")";
   return res;
}

std::unique_ptr<operators::Operator> And::compile(
   const Database& database,
   const DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<operators::Operator>> all_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(all_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(database, database_partition);
      }
   );
   for (auto& child : all_child_operators) {
      if (child->type() == operators::FULL) {
         continue;
      }
      if (child->type() == operators::EMPTY) {
         return std::make_unique<operators::Empty>();
      }
      if (child->type() == operators::INTERSECTION) {
         auto* intersection_child = dynamic_cast<operators::Intersection*>(child.get());
         std::transform(
            intersection_child->children.begin(),
            intersection_child->children.end(),
            std::back_inserter(non_negated_child_operators),
            [&](std::unique_ptr<operators::Operator>& expression) { return std::move(expression); }
         );
         std::transform(
            intersection_child->negated_children.begin(),
            intersection_child->negated_children.end(),
            std::back_inserter(negated_child_operators),
            [&](std::unique_ptr<operators::Operator>& expression) { return std::move(expression); }
         );
      } else if (child->type() == operators::COMPLEMENT) {
         auto* negated_child = dynamic_cast<operators::Complement*>(child.get());
         negated_child_operators.emplace_back(std::move(negated_child->child));
      } else {
         non_negated_child_operators.push_back(std::move(child));
      }
   }
   if (non_negated_child_operators.empty() && negated_child_operators.empty()) {
      return std::make_unique<operators::Full>(database_partition.sequenceCount);
   }
   if (non_negated_child_operators.size() == 1 && negated_child_operators.empty()) {
      return std::move(non_negated_child_operators[0]);
   }
   if (negated_child_operators.size() == 1 && non_negated_child_operators.empty()) {
      return std::make_unique<operators::Complement>(
         std::move(negated_child_operators[0]), database_partition.sequenceCount
      );
   }
   if (non_negated_child_operators.empty()) {
      std::unique_ptr<operators::Union> union_ret =
         std::make_unique<operators::Union>(std::move(negated_child_operators));
      return std::make_unique<operators::Complement>(
         std::move(union_ret), database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::Intersection>(
      std::move(non_negated_child_operators), std::move(negated_child_operators)
   );
}

}  // namespace silo::query_engine::filter_expressions
