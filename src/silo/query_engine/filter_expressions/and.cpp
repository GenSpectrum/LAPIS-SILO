#include "silo/query_engine/filter_expressions/and.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/union.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo {
struct Database;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

using OperatorVector = std::vector<std::unique_ptr<operators::Operator>>;

And::And(std::vector<std::unique_ptr<Expression>>&& children)
    : children(std::move(children)) {}

std::string And::toString(const silo::Database& database) const {
   std::vector<std::string> child_strings;
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(child_strings),
      [&](const std::unique_ptr<Expression>& child) { return child->toString(database); }
   );
   std::string res = "(" + boost::algorithm::join(child_strings, " & ") + ")";
   return res;
}

namespace {

void inline appendVectorToVector(OperatorVector& vec_1, OperatorVector& vec_2) {
   std::transform(
      vec_1.begin(),
      vec_1.end(),
      std::back_inserter(vec_2),
      [&](std::unique_ptr<operators::Operator>& ele) { return std::move(ele); }
   );
}
}  // namespace

std::tuple<OperatorVector, OperatorVector> And::compileChildren(
   const Database& database,
   const DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   OperatorVector all_child_operators;
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(all_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(database, database_partition, mode);
      }
   );
   OperatorVector non_negated_child_operators;
   OperatorVector negated_child_operators;
   for (auto& child : all_child_operators) {
      if (child->type() == operators::FULL) {
         continue;
      }
      if (child->type() == operators::EMPTY) {
         OperatorVector empty;
         empty.emplace_back(std::make_unique<operators::Empty>(database_partition.sequenceCount));
         return {std::move(empty), OperatorVector()};
      }
      if (child->type() == operators::INTERSECTION) {
         auto* intersection_child = dynamic_cast<operators::Intersection*>(child.get());
         appendVectorToVector(intersection_child->children, non_negated_child_operators);
         appendVectorToVector(intersection_child->negated_children, negated_child_operators);
      } else if (child->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(child->negate());
      } else {
         non_negated_child_operators.push_back(std::move(child));
      }
   }
   return {std::move(non_negated_child_operators), std::move(negated_child_operators)};
}

std::unique_ptr<operators::Operator> And::compile(
   const Database& database,
   const DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   OperatorVector non_negated_child_operators;
   OperatorVector negated_child_operators;
   std::tie(non_negated_child_operators, negated_child_operators) =
      compileChildren(database, database_partition, mode);

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
      std::unique_ptr<operators::Union> union_ret = std::make_unique<operators::Union>(
         std::move(negated_child_operators), database_partition.sequenceCount
      );
      return std::make_unique<operators::Complement>(
         std::move(union_ret), database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::Intersection>(
      std::move(non_negated_child_operators),
      std::move(negated_child_operators),
      database_partition.sequenceCount
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<And>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an And expression"
   )
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an And expression needs to be an array"
   )
   auto children = json.at("children").get<std::vector<std::unique_ptr<Expression>>>();
   filter = std::make_unique<And>(std::move(children));
}

}  // namespace silo::query_engine::filter_expressions
