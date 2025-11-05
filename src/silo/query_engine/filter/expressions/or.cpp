#include "silo/query_engine/filter/expressions/or.h"

#include <algorithm>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/storage/table_partition.h"
#include "symbol_in_set.h"

namespace silo::query_engine::filter::expressions {

using operators::OperatorVector;

Or::Or(ExpressionVector&& children)
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

template <typename SymbolType>
ExpressionVector Or::rewriteSymbolInSetExpressions(ExpressionVector children) {
   ExpressionVector new_children;
   using SequenceNameAndPosition = std::pair<std::optional<std::string>, uint32_t>;
   using Symbols = std::vector<typename SymbolType::Symbol>;
   std::map<SequenceNameAndPosition, Symbols> symbol_in_set_children;
   for (auto& child : children) {
      if (auto symbol_in_set_child = dynamic_cast<SymbolInSet<SymbolType>*>(child.get());
          symbol_in_set_child != nullptr) {
         std::vector<typename SymbolType::Symbol>& symbols_so_far = symbol_in_set_children[{
            symbol_in_set_child->sequence_name, symbol_in_set_child->position_idx
         }];
         std::ranges::copy(symbol_in_set_child->symbols, std::back_inserter(symbols_so_far));
      } else {
         new_children.emplace_back(std::move(child));
      }
   }

   for (auto& [sequence_name_and_position, symbols] : symbol_in_set_children) {
      new_children.emplace_back(std::make_unique<SymbolInSet<SymbolType>>(
         sequence_name_and_position.first, sequence_name_and_position.second, std::move(symbols)
      ));
   }

   return new_children;
}

std::unique_ptr<Expression> Or::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   ExpressionVector rewritten_children;
   std::ranges::transform(
      children,
      std::back_inserter(rewritten_children),
      [&](const std::unique_ptr<Expression>& child) {
         return child->rewrite(table, table_partition, mode);
      }
   );
   rewritten_children = rewriteSymbolInSetExpressions<Nucleotide>(std::move(rewritten_children));
   rewritten_children = rewriteSymbolInSetExpressions<AminoAcid>(std::move(rewritten_children));
   if (rewritten_children.size() == 1) {
      return std::move(rewritten_children[0]);
   }
   return std::make_unique<Or>(std::move(rewritten_children));
}

std::unique_ptr<operators::Operator> Or::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
) const {
   OperatorVector all_child_operators;
   std::ranges::transform(
      children,
      std::back_inserter(all_child_operators),
      [&](const std::unique_ptr<Expression>& expression) {
         return expression->compile(table, table_partition);
      }
   );
   OperatorVector filtered_child_operators;
   for (auto& child : all_child_operators) {
      if (child->type() == operators::EMPTY) {
         continue;
      }
      if (child->type() == operators::FULL) {
         return std::make_unique<operators::Full>(table_partition.sequence_count);
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
      return std::make_unique<operators::Empty>(table_partition.sequence_count);
   }
   if (filtered_child_operators.size() == 1) {
      return std::move(filtered_child_operators[0]);
   }

   if (std::ranges::any_of(filtered_child_operators, [](const auto& child) {
          return child->type() == operators::COMPLEMENT;
       })) {
      return operators::Complement::fromDeMorgan(
         std::move(filtered_child_operators), table_partition.sequence_count
      );
   }
   return std::make_unique<operators::Union>(
      std::move(filtered_child_operators), table_partition.sequence_count
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
   auto children = json["children"].get<ExpressionVector>();
   filter = std::make_unique<Or>(std::move(children));
}

}  // namespace silo::query_engine::filter::expressions
