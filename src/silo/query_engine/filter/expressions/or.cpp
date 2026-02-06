#include "silo/query_engine/filter/expressions/or.h"

#include <algorithm>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/table_partition.h"

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

std::vector<const Expression*> Or::collectChildren(const ExpressionVector& children) {
   std::vector<const Expression*> result;

   std::vector<const Expression*> queue;
   for (auto& direct_child : children) {
      queue.push_back(direct_child.get());
   }

   while (!queue.empty()) {
      auto current = queue.back();
      queue.pop_back();
      if (dynamic_cast<const Or*>(current) != nullptr) {
         const Or* or_child = dynamic_cast<const Or*>(current);
         for (auto& child : or_child->children) {
            queue.push_back(child.get());
         }
      } else {
         result.push_back(current);
      }
   }
   return result;
}

ExpressionVector Or::algebraicSimplification(ExpressionVector unprocessed_child_expressions) {
   ExpressionVector non_trivial_children;
   while (!unprocessed_child_expressions.empty()) {
      auto child = std::move(unprocessed_child_expressions.back());
      unprocessed_child_expressions.pop_back();
      if (dynamic_cast<False*>(child.get()) != nullptr) {
         SPDLOG_TRACE("Skipping 'False' child");
         continue;
      }
      if (dynamic_cast<True*>(child.get()) != nullptr) {
         SPDLOG_TRACE("Shortcutting because found 'True' child");
         ExpressionVector singleton_true;
         singleton_true.emplace_back(std::make_unique<True>());
         return singleton_true;
      }
      if (dynamic_cast<Or*>(child.get()) != nullptr) {
         auto* or_child = dynamic_cast<Or*>(child.get());
         appendVectorToVector(or_child->children, unprocessed_child_expressions);
      } else {
         non_trivial_children.push_back(std::move(child));
      }
   }
   return non_trivial_children;
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

template ExpressionVector Or::rewriteSymbolInSetExpressions<AminoAcid>(ExpressionVector children);
template ExpressionVector Or::rewriteSymbolInSetExpressions<Nucleotide>(ExpressionVector children);

ExpressionVector Or::mergeStringInSetExpressions(ExpressionVector children) {
   ExpressionVector new_children;
   using Column = std::string;
   using Strings = std::unordered_set<std::string>;
   std::map<Column, Strings> string_in_set_children;
   for (auto& child : children) {
      if (auto string_in_set_child = dynamic_cast<StringInSet*>(child.get());
          string_in_set_child != nullptr) {
         if (auto it = string_in_set_children.find(string_in_set_child->column_name);
             it != string_in_set_children.end()) {
            Strings child_strings = std::move(string_in_set_child->values);
            for (auto& value : child_strings) {
               it->second.emplace(std::move(value));
            }
         } else {
            string_in_set_children.emplace(
               std::move(string_in_set_child->column_name), std::move(string_in_set_child->values)
            );
         }
      } else {
         new_children.emplace_back(std::move(child));
      }
   }

   for (auto& [column_name, strings] : string_in_set_children) {
      new_children.emplace_back(
         std::make_unique<StringInSet>(std::move(column_name), std::move(strings))
      );
   }

   return new_children;
}

std::unique_ptr<Expression> Or::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   std::vector<const Expression*> collected_children = collectChildren(children);
   ExpressionVector rewritten_children;
   std::ranges::transform(
      collected_children,
      std::back_inserter(rewritten_children),
      [&](const Expression* child) { return child->rewrite(table, table_partition, mode); }
   );
   rewritten_children = algebraicSimplification(std::move(rewritten_children));
   rewritten_children = rewriteSymbolInSetExpressions<Nucleotide>(std::move(rewritten_children));
   rewritten_children = rewriteSymbolInSetExpressions<AminoAcid>(std::move(rewritten_children));
   rewritten_children = mergeStringInSetExpressions(std::move(rewritten_children));
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
