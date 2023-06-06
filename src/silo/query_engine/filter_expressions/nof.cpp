#include "silo/query_engine/filter_expressions/nof.h"

#include <nlohmann/json.hpp>
#include <vector>

#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/threshold.h"
#include "silo/query_engine/operators/union.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace {

using Operator = silo::query_engine::operators::Operator;
using Empty = silo::query_engine::operators::Empty;
using Full = silo::query_engine::operators::Full;
using Complement = silo::query_engine::operators::Complement;
using Intersection = silo::query_engine::operators::Intersection;
using Union = silo::query_engine::operators::Union;
using Threshold = silo::query_engine::operators::Threshold;

std::unique_ptr<Operator> handleTrivialCases(
   const int updated_number_of_matchers,
   std::vector<std::unique_ptr<Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<Operator>>& negated_child_operators,
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
   std::vector<std::unique_ptr<Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<Operator>>& negated_child_operators,
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
   std::vector<std::unique_ptr<Operator>>& non_negated_child_operators,
   std::vector<std::unique_ptr<Operator>>& negated_child_operators,
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
   std::vector<std::unique_ptr<Operator>>&& non_negated_child_operators,
   std::vector<std::unique_ptr<Operator>>&& negated_child_operators,
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
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
   int updated_number_of_matchers = number_of_matchers;

   for (const auto& child_expression : children) {
      auto child_operator = child_expression->compile(database, database_partition, mode);
      if (child_operator->type() == operators::EMPTY) {
         continue;
      }
      if (child_operator->type() == operators::FULL) {
         updated_number_of_matchers--;
      } else if (child_operator->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(child_operator->negate());
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

std::unique_ptr<operators::Operator> NOf::rewriteNonExact(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   std::vector<std::unique_ptr<operators::Operator>> at_least_k;
   {
      int updated_number_of_matchers;
      std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
      std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
      std::tie(non_negated_child_operators, negated_child_operators, updated_number_of_matchers) =
         mapChildExpressions(database, database_partition, mode);
      at_least_k.emplace_back(toOperator(
         updated_number_of_matchers,
         std::move(non_negated_child_operators),
         std::move(negated_child_operators),
         false,
         database_partition.sequenceCount
      ));
   }

   std::vector<std::unique_ptr<operators::Operator>> at_least_k_plus_one;
   {
      int updated_number_of_matchers;
      std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
      std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
      std::tie(non_negated_child_operators, negated_child_operators, updated_number_of_matchers) =
         mapChildExpressions(database, database_partition, mode);
      at_least_k_plus_one.emplace_back(toOperator(
         updated_number_of_matchers + 1,
         std::move(non_negated_child_operators),
         std::move(negated_child_operators),
         false,
         database_partition.sequenceCount
      ));
   }

   return toOperator(
      2,
      std::move(at_least_k),
      std::move(at_least_k_plus_one),
      false,
      database_partition.sequenceCount
   );
}

std::unique_ptr<operators::Operator> NOf::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   int updated_number_of_matchers;
   std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
   std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
   std::tie(non_negated_child_operators, negated_child_operators, updated_number_of_matchers) =
      mapChildExpressions(database, database_partition, mode);

   // We cannot easily map ambiguity modes through an exact NOf expression -> rewrite without exact
   if (mode != NONE && match_exactly && number_of_matchers < children.size()) {
      return rewriteNonExact(database, database_partition, mode);
   }

   return toOperator(
      updated_number_of_matchers,
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      match_exactly,
      database_partition.sequenceCount
   );
}

void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an N-Of expression"
   )
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an N-Of expression needs to be an array"
   )
   CHECK_SILO_QUERY(
      json.contains("numberOfMatchers"),
      "The field 'numberOfMatchers' is required in an N-Of expression"
   )
   CHECK_SILO_QUERY(
      json["numberOfMatchers"].is_number_unsigned(),
      "The field 'numberOfMatchers' in an N-Of expression needs to be an unsigned integer"
   )
   CHECK_SILO_QUERY(
      json.contains("matchExactly"), "The field 'matchExactly' is required in an N-Of expression"
   )
   CHECK_SILO_QUERY(
      json["matchExactly"].is_boolean(),
      "The field 'matchExactly' in an N-Of expression needs to be a boolean"
   )

   const unsigned number_of_matchers = json["numberOfMatchers"];
   const bool match_exactly = json["matchExactly"];
   auto children = json["children"].get<std::vector<std::unique_ptr<Expression>>>();
   filter = std::make_unique<NOf>(std::move(children), number_of_matchers, match_exactly);
}

}  // namespace silo::query_engine::filter_expressions