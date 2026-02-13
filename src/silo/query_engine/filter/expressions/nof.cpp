#include "silo/query_engine/filter/expressions/nof.h"

#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/intersection.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/threshold.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/table_partition.h"

namespace {

using Operator = silo::query_engine::filter::operators::Operator;
using OperatorVector = silo::query_engine::filter::operators::OperatorVector;
using Empty = silo::query_engine::filter::operators::Empty;
using Full = silo::query_engine::filter::operators::Full;
using Complement = silo::query_engine::filter::operators::Complement;
using Intersection = silo::query_engine::filter::operators::Intersection;
using Union = silo::query_engine::filter::operators::Union;
using Threshold = silo::query_engine::filter::operators::Threshold;

std::unique_ptr<Operator> handleTrivialCases(
   const int updated_number_of_matchers,
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   bool match_exactly,
   uint32_t sequence_count
) {
   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers > child_operator_count) {
      return std::make_unique<Empty>(sequence_count);
   }
   if (updated_number_of_matchers < 0) {
      if (match_exactly) {
         return std::make_unique<Empty>(sequence_count);
      }
      return std::make_unique<Full>(sequence_count);
   }
   if (updated_number_of_matchers == 0) {
      if (!match_exactly) {
         return std::make_unique<Full>(sequence_count);
      }
      /// Now we want to match exactly none
      if (child_operator_count == 0) {
         return std::make_unique<Full>(sequence_count);
      }
      if (child_operator_count == 1) {
         if (non_negated_child_operators.empty()) {
            return std::move(negated_child_operators[0]);
         }
         return std::make_unique<Complement>(
            std::move(non_negated_child_operators[0]), sequence_count
         );
      }
      /// To negate entire result Not(Union) => Intersection(Not(Non-negated),Not(Negated))
      /// equiv: Intersection(Negated, Non-Negated) or Not(Union(Non-negated)), if negated empty
      if (negated_child_operators.empty()) {
         auto union_ret =
            std::make_unique<Union>(std::move(non_negated_child_operators), sequence_count);
         return std::make_unique<Complement>(std::move(union_ret), sequence_count);
      }
      return std::make_unique<Intersection>(
         std::move(negated_child_operators), std::move(non_negated_child_operators), sequence_count
      );
   }
   if (updated_number_of_matchers == 1 && child_operator_count == 1) {
      if (negated_child_operators.empty()) {
         return std::move(non_negated_child_operators[0]);
      }
      return std::make_unique<Complement>(std::move(negated_child_operators[0]), sequence_count);
   }
   return nullptr;
}

std::unique_ptr<Operator> handleAndCase(
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   uint32_t sequence_count
) {
   if (non_negated_child_operators.empty()) {
      std::unique_ptr<Union> union_ret =
         std::make_unique<Union>(std::move(negated_child_operators), sequence_count);
      return std::make_unique<Complement>(std::move(union_ret), sequence_count);
   }
   return std::make_unique<Intersection>(
      std::move(non_negated_child_operators), std::move(negated_child_operators), sequence_count
   );
}

std::unique_ptr<Operator> handleOrCase(
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   uint32_t sequence_count
) {
   if (negated_child_operators.empty()) {
      return std::make_unique<Union>(std::move(non_negated_child_operators), sequence_count);
   }
   /// De'Morgan if at least one negated
   std::unique_ptr<Intersection> intersection_ret = std::make_unique<Intersection>(
      std::move(negated_child_operators), std::move(non_negated_child_operators), sequence_count
   );
   return std::make_unique<Complement>(std::move(intersection_ret), sequence_count);
}

std::unique_ptr<Operator> toOperator(
   const int updated_number_of_matchers,
   OperatorVector&& non_negated_child_operators,
   OperatorVector&& negated_child_operators,
   bool match_exactly,
   uint32_t sequence_count
) {
   auto tmp = handleTrivialCases(
      updated_number_of_matchers,
      non_negated_child_operators,
      negated_child_operators,
      match_exactly,
      sequence_count
   );
   if (tmp) {
      return tmp;
   }

   const int child_operator_count =
      static_cast<int>(non_negated_child_operators.size() + negated_child_operators.size());

   if (updated_number_of_matchers == child_operator_count) {
      return handleAndCase(non_negated_child_operators, negated_child_operators, sequence_count);
   }
   if (updated_number_of_matchers == 1 && !match_exactly) {
      return handleOrCase(non_negated_child_operators, negated_child_operators, sequence_count);
   }
   return std::make_unique<Threshold>(
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers,
      match_exactly,
      sequence_count
   );
}

}  // namespace

namespace silo::query_engine::filter::expressions {

NOf::NOf(ExpressionVector&& children, int number_of_matchers, bool match_exactly)
    : children(std::move(children)),
      number_of_matchers(number_of_matchers),
      match_exactly(match_exactly) {}

std::string NOf::toString() const {
   std::string res;
   if (match_exactly) {
      res = "[exactly-" + std::to_string(number_of_matchers) + "-of:";
   } else {
      res = "[" + std::to_string(number_of_matchers) + "-of:";
   }
   for (const auto& child : children) {
      res += child->toString();
      res += ", ";
   }
   res += "]";
   return res;
}

std::tuple<operators::OperatorVector, operators::OperatorVector, int> NOf::mapChildExpressions(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   operators::OperatorVector child_operators;
   child_operators.reserve(children.size());
   for (const auto& child_expression : children) {
      child_operators.push_back(child_expression->compile(table, table_partition));
   }

   operators::OperatorVector non_negated_child_operators;
   operators::OperatorVector negated_child_operators;
   int updated_number_of_matchers = number_of_matchers;

   for (auto& child_operator : child_operators) {
      if (child_operator->type() == operators::EMPTY) {
         continue;
      }
      if (child_operator->type() == operators::FULL) {
         updated_number_of_matchers--;
         continue;
      }
      if (child_operator->type() == operators::COMPLEMENT) {
         auto canceled_negation = operators::Operator::negate(std::move(child_operator));
         negated_child_operators.emplace_back(std::move(canceled_negation));
         continue;
      }
      non_negated_child_operators.push_back(std::move(child_operator));
   }
   return std::tuple<operators::OperatorVector, operators::OperatorVector, int>{
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      updated_number_of_matchers
   };
}

ExpressionVector NOf::rewriteChildren(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   ExpressionVector rewritten_children;
   rewritten_children.reserve(children.size());
   for (const auto& child : children) {
      rewritten_children.push_back(child->rewrite(table, table_partition, mode));
   }
   return rewritten_children;
}

std::unique_ptr<Expression> NOf::rewriteToNonExact(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   auto at_least_k = std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode),
      this->number_of_matchers,
      /*match_exactly=*/false
   );
   auto at_least_k_plus_one = std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode),
      this->number_of_matchers + 1,
      /*match_exactly=*/false
   );
   ;
   ExpressionVector and_children;
   and_children.push_back(std::move(at_least_k));
   and_children.push_back(std::make_unique<Negation>(std::move(at_least_k_plus_one)));
   return std::make_unique<And>(std::move(and_children));
}

std::unique_ptr<Expression> NOf::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
) const {
   // We cannot easily map ambiguity modes through an exact NOf expression -> rewrite without exact
   if (mode != NONE && match_exactly && std::cmp_less(number_of_matchers, children.size())) {
      return rewriteToNonExact(table, table_partition, mode);
   }

   return std::make_unique<NOf>(
      rewriteChildren(table, table_partition, mode), number_of_matchers, match_exactly
   );
}

std::unique_ptr<operators::Operator> NOf::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   auto [non_negated_child_operators, negated_child_operators, updated_number_of_matchers] =
      mapChildExpressions(table, table_partition);

   return toOperator(
      updated_number_of_matchers,
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      match_exactly,
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an N-Of expression needs to be an array"
   );
   CHECK_SILO_QUERY(
      json.contains("numberOfMatchers"),
      "The field 'numberOfMatchers' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["numberOfMatchers"].is_number_unsigned(),
      "The field 'numberOfMatchers' in an N-Of expression needs to be an unsigned integer"
   );
   CHECK_SILO_QUERY(
      json.contains("matchExactly"), "The field 'matchExactly' is required in an N-Of expression"
   );
   CHECK_SILO_QUERY(
      json["matchExactly"].is_boolean(),
      "The field 'matchExactly' in an N-Of expression needs to be a boolean"
   );

   const uint32_t number_of_matchers = json["numberOfMatchers"];
   const bool match_exactly = json["matchExactly"];
   auto children = json["children"].get<ExpressionVector>();
   filter = std::make_unique<NOf>(std::move(children), number_of_matchers, match_exactly);
}

}  // namespace silo::query_engine::filter::expressions
