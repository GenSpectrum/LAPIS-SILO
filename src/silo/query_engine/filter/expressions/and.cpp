#include "silo/query_engine/filter/expressions/and.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/intersection.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using operators::Operator;
using operators::OperatorVector;

And::And(ExpressionVector&& children)
    : children(std::move(children)) {}

std::string And::toString() const {
   std::vector<std::string> child_strings;
   std::ranges::transform(
      children,
      std::back_inserter(child_strings),
      [&](const std::unique_ptr<Expression>& child) { return child->toString(); }
   );
   return "And(" + boost::algorithm::join(child_strings, " & ") + ")";
}

namespace {

template <typename T>
void inline appendVectorToVector(
   std::vector<std::unique_ptr<T>>& vec_1,
   std::vector<std::unique_ptr<T>>& vec_2
) {
   std::ranges::transform(vec_1, std::back_inserter(vec_2), [&](std::unique_ptr<T>& ele) {
      return std::move(ele);
   });
}

void logCompiledChildren(
   OperatorVector& non_negated_child_operators,
   OperatorVector& negated_child_operators,
   operators::PredicateVector& predicates
) {
   std::vector<std::string> child_operator_strings;
   std::ranges::transform(
      non_negated_child_operators,
      std::back_inserter(child_operator_strings),
      [&](const std::unique_ptr<operators::Operator>& operator_) { return operator_->toString(); }
   );
   std::ranges::transform(
      negated_child_operators,
      std::back_inserter(child_operator_strings),
      [&](const std::unique_ptr<operators::Operator>& operator_) {
         return "!" + operator_->toString();
      }
   );
   std::vector<std::string> predicate_strings;
   std::ranges::transform(
      predicates,
      std::back_inserter(predicate_strings),
      [&](const std::unique_ptr<operators::Predicate>& predicate) { return predicate->toString(); }
   );
   SPDLOG_TRACE(
      "Compiled and processed child operators: {}, predicates {}, children: {}, negated children: "
      "{}, predicates: {}",
      fmt::join(child_operator_strings, ","),
      fmt::join(predicate_strings, ","),
      non_negated_child_operators.size(),
      negated_child_operators.size(),
      predicates.size()
   );
}
}  // namespace

std::tuple<OperatorVector, OperatorVector, operators::PredicateVector> And::compileChildren(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   OperatorVector unprocessed_child_operators;
   std::ranges::transform(
      children,
      std::back_inserter(unprocessed_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(table, table_partition);
      }
   );
   OperatorVector non_negated_child_operators;
   OperatorVector negated_child_operators;
   operators::PredicateVector predicates;
   while (!unprocessed_child_operators.empty()) {
      auto child = std::move(unprocessed_child_operators.back());
      unprocessed_child_operators.pop_back();
      if (child->type() == operators::FULL) {
         SPDLOG_TRACE("Skipping full child");
         continue;
      }
      if (child->type() == operators::EMPTY) {
         SPDLOG_TRACE("Shortcutting because found empty child");
         OperatorVector empty;
         empty.emplace_back(std::make_unique<operators::Empty>(table_partition.sequence_count));
         return {std::move(empty), OperatorVector(), operators::PredicateVector{}};
      }
      if (child->type() == operators::INTERSECTION) {
         auto* intersection_child = dynamic_cast<operators::Intersection*>(child.get());
         appendVectorToVector(intersection_child->children, non_negated_child_operators);
         appendVectorToVector(intersection_child->negated_children, negated_child_operators);
      } else if (child->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(operators::Operator::negate(std::move(child)));
      } else if (child->type() == operators::SELECTION) {
         auto* selection_child = dynamic_cast<operators::Selection*>(child.get());
         appendVectorToVector<operators::Predicate>(selection_child->predicates, predicates);
         SPDLOG_TRACE(
            "Found selection, appended {} predicates", selection_child->predicates.size()
         );
         if (selection_child->child_operator.has_value()) {
            auto child_operator = std::move(selection_child->child_operator.value());
            SPDLOG_TRACE("Appending child of selection {}", child_operator->toString());
            unprocessed_child_operators.emplace_back(std::move(child_operator));
         }
      } else {
         non_negated_child_operators.push_back(std::move(child));
      }
   }

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
   logCompiledChildren(non_negated_child_operators, negated_child_operators, predicates);
#endif

   return {
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      std::move(predicates)
   };
}

std::unique_ptr<Expression> And::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
) const {
   ExpressionVector rewritten_children;
   rewritten_children.reserve(children.size());
   for (const auto& child : children) {
      rewritten_children.emplace_back(child->rewrite(table, table_partition, mode));
   }
   return std::make_unique<And>(std::move(rewritten_children));
}

std::unique_ptr<Operator> And::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   auto [non_negated_child_operators, negated_child_operators, predicates] =
      compileChildren(table, table_partition);

   if (non_negated_child_operators.empty() && negated_child_operators.empty()) {
      if (predicates.empty()) {
         SPDLOG_TRACE(
            "Compiled And filter expression to Full, since no predicates and no child operators"
         );
         return std::make_unique<operators::Full>(table_partition.sequence_count);
      }
      auto result = std::make_unique<operators::Selection>(
         std::move(predicates), table_partition.sequence_count
      );
      SPDLOG_TRACE(
         "Compiled And filter expression to {} - found only predicates", result->toString()
      );

      return result;
   }

   std::unique_ptr<Operator> index_arithmetic_operator;
   if (non_negated_child_operators.size() == 1 && negated_child_operators.empty()) {
      index_arithmetic_operator = std::move(non_negated_child_operators[0]);
   } else if (negated_child_operators.size() == 1 && non_negated_child_operators.empty()) {
      index_arithmetic_operator = std::make_unique<operators::Complement>(
         std::move(negated_child_operators[0]), table_partition.sequence_count
      );
   } else if (non_negated_child_operators.empty()) {
      std::unique_ptr<operators::Union> union_ret = std::make_unique<operators::Union>(
         std::move(negated_child_operators), table_partition.sequence_count
      );
      index_arithmetic_operator = std::make_unique<operators::Complement>(
         std::move(union_ret), table_partition.sequence_count
      );
   } else {
      index_arithmetic_operator = std::make_unique<operators::Intersection>(
         std::move(non_negated_child_operators),
         std::move(negated_child_operators),
         table_partition.sequence_count
      );
   }
   if (predicates.empty()) {
      SPDLOG_TRACE(
         "Compiled And filter expression to {} - found no predicates",
         index_arithmetic_operator->toString()
      );

      return index_arithmetic_operator;
   }
   auto result = std::make_unique<operators::Selection>(
      std::move(index_arithmetic_operator), std::move(predicates), table_partition.sequence_count
   );

   SPDLOG_TRACE("Compiled And filter expression to {}", result->toString());

   return result;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<And>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an And expression"
   );
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an And expression needs to be an array"
   );
   auto children = json.at("children").get<ExpressionVector>();
   filter = std::make_unique<And>(std::move(children));
}

}  // namespace silo::query_engine::filter::expressions
