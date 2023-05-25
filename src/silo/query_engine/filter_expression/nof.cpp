#include "silo/query_engine/filter_expressions/nof.h"

#include <vector>

#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/threshold.h"
#include "silo/query_engine/operators/union.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

NOf::NOf(
   std::vector<std::unique_ptr<Expression>>&& children,
   int number_of_matchers,
   bool match_exactly
)
    : children(std::move(children)),
      number_of_matchers(number_of_matchers),
      match_exactly(match_exactly) {}

std::string NOf::toString(const silo::Database& database) {
   std::string res;
   if (match_exactly) {
      res = "[exactly-" + std::to_string(number_of_matchers) + "-of:";
   } else {
      res = "[" + std::to_string(number_of_matchers) + "-of:";
   }
   for (auto& child : children) {
      res += child->toString(database);
      res += ", ";
   }
   res += "]";
   return res;
}

std::tuple<
   std::vector<std::unique_ptr<operators::Operator>>,
   std::vector<std::unique_ptr<operators::Operator>>,
   int>
NOf::mapChildExpressions(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
   int updated_number_of_matchers = number_of_matchers;

   for (const auto& child_expression : children) {
      auto child_operator = child_expression->compile(database, database_partition);
      if (child_operator->type() == operators::EMPTY) {
         continue;
      }
      if (child_operator->type() == operators::FULL) {
         updated_number_of_matchers--;
      } else if (child_operator->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(
            std::move(dynamic_cast<operators::Complement*>(child_operator.get())->child)
         );
      } else {
         non_negated_child_operators.push_back(std::move(child_operator));
      }
   }
   return std::tuple<
      std::vector<std::unique_ptr<operators::Operator>>,
      std::vector<std::unique_ptr<operators::Operator>>,
      int>{
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers};
}

namespace {
std::unique_ptr<silo::query_engine::operators::Operator> handleTrivialCases(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   const int updated_number_of_matchers,
   std::vector<std::unique_ptr<operators::Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<operators::Operator>>& negated_child_operators,
   bool match_exactly
) {
   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers > child_operator_count) {
      return std::make_unique<operators::Empty>();
   }
   if (updated_number_of_matchers < 0) {
      if (match_exactly) {
         return std::make_unique<operators::Empty>();
      }
      return std::make_unique<operators::Full>(database_partition.sequenceCount);
   }
   if (updated_number_of_matchers == 0) {
      if (!match_exactly) {
         return std::make_unique<operators::Full>(database_partition.sequenceCount);
      }
      /// Now we want to match exactly none
      if (child_operator_count == 0) {
         return std::make_unique<operators::Full>(database_partition.sequenceCount);
      }
      if (child_operator_count == 1) {
         if (non_negated_child_operators.empty()) {
            return std::move(negated_child_operators[0]);
         }
         return std::make_unique<operators::Complement>(
            std::move(non_negated_child_operators[0]), database_partition.sequenceCount
         );
      }
      /// To negate entire result Not(Union) => Intersection(Not(Non-negated),Not(Negated))
      /// equiv: Intersection(Negated, Non-Negated) or Not(Union(Non-negated)), if negated empty
      if (negated_child_operators.empty()) {
         auto union_ret =
            std::make_unique<operators::Union>(std::move(non_negated_child_operators));
         return std::make_unique<operators::Complement>(
            std::move(union_ret), database_partition.sequenceCount
         );
      }
      return std::make_unique<operators::Intersection>(
         std::move(negated_child_operators), std::move(non_negated_child_operators)
      );
   }
   if (updated_number_of_matchers == 1 && child_operator_count == 1) {
      if (negated_child_operators.empty()) {
         return std::move(non_negated_child_operators[0]);
      }
      return std::make_unique<operators::Complement>(
         std::move(negated_child_operators[0]), database_partition.sequenceCount
      );
   }
   return nullptr;
}

std::unique_ptr<operators::Operator> handleAndCase(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   std::vector<std::unique_ptr<operators::Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<operators::Operator>>& negated_child_operators
) {
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

std::unique_ptr<operators::Operator> handleOrCase(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   std::vector<std::unique_ptr<operators::Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<operators::Operator>>& negated_child_operators
) {
   if (negated_child_operators.empty()) {
      return std::make_unique<operators::Union>(std::move(non_negated_child_operators));
   }
   /// De'Morgan if at least one negated
   std::unique_ptr<operators::Intersection> intersection_ret =
      std::make_unique<operators::Intersection>(
         std::move(negated_child_operators), std::move(non_negated_child_operators)
      );
   return std::make_unique<operators::Complement>(
      std::move(intersection_ret), database_partition.sequenceCount
   );
}

}  // namespace

std::unique_ptr<operators::Operator> NOf::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   int updated_number_of_matchers;
   std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
   std::tie(non_negated_child_operators, negated_child_operators, updated_number_of_matchers) =
      mapChildExpressions(database, database_partition);

   auto tmp = handleTrivialCases(
      database,
      database_partition,
      updated_number_of_matchers,
      non_negated_child_operators,
      negated_child_operators,
      match_exactly
   );
   if (tmp) {
      return tmp;
   }

   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers == child_operator_count) {
      return handleAndCase(
         database, database_partition, non_negated_child_operators, negated_child_operators
      );
   }
   if (updated_number_of_matchers == 1 && !match_exactly) {
      return handleOrCase(
         database, database_partition, non_negated_child_operators, negated_child_operators
      );
   }
   return std::make_unique<operators::Threshold>(
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers,
      match_exactly,
      database_partition.sequenceCount
   );
}

}  // namespace silo::query_engine::filter_expressions