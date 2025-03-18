#include "silo/query_engine/filter/expressions/or.h"

#include <algorithm>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using OperatorVector = std::vector<std::unique_ptr<operators::Operator>>;

Or::Or(std::vector<std::unique_ptr<Expression>>&& children)
    : children(std::move(children)) {}

std::string Or::toString() const {
   std::vector<std::string> child_strings;
   std::ranges::transform(
      children,
      std::back_inserter(child_strings),
      [&](const std::unique_ptr<Expression>& child) { return child->toString(); }
   );
   return "Or(" + boost::algorithm::join(child_strings, " | ") + ")";
}

std::unique_ptr<operators::Operator> Or::compile(
   const Database& database,
   const storage::TablePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   OperatorVector all_child_operators;
   std::ranges::transform(
      children,
      std::back_inserter(all_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(database, database_partition, mode);
      }
   );
   OperatorVector filtered_child_operators;
   for (auto& child : all_child_operators) {
      if (child->type() == operators::EMPTY) {
         continue;
      }
      if (child->type() == operators::FULL) {
         return std::make_unique<operators::Full>(database_partition.sequence_count);
      }
      if (child->type() == operators::UNION) {
         auto* or_child = dynamic_cast<operators::Union*>(child.get());
         std::ranges::transform(
            or_child->children,
            std::back_inserter(filtered_child_operators),
            [&](std::unique_ptr<operators::Operator>& expression) { return std::move(expression); }
         );
      } else {
         filtered_child_operators.push_back(std::move(child));
      }
   }
   if (filtered_child_operators.empty()) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }
   if (filtered_child_operators.size() == 1) {
      return std::move(filtered_child_operators[0]);
   }

   if (std::ranges::any_of(filtered_child_operators, [](const auto& child) {
          return child->type() == operators::COMPLEMENT;
       })) {
      return operators::Complement::fromDeMorgan(
         std::move(filtered_child_operators), database_partition.sequence_count
      );
   }
   return std::make_unique<operators::Union>(
      std::move(filtered_child_operators), database_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Or>& filter) {
   CHECK_SILO_QUERY(
      json.contains("children"), "The field 'children' is required in an Or expression"
   );
   CHECK_SILO_QUERY(
      json["children"].is_array(), "The field 'children' in an Or expression needs to be an array"
   );
   auto children = json["children"].get<std::vector<std::unique_ptr<Expression>>>();
   filter = std::make_unique<Or>(std::move(children));
}

}  // namespace silo::query_engine::filter::expressions
