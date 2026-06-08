#include "silo/query_engine/saneql/ast_to_query.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <re2/re2.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/and.h"
#include "silo/query_engine/expressions/bool_equals.h"
#include "silo/query_engine/expressions/date_between.h"
#include "silo/query_engine/expressions/date_equals.h"
#include "silo/query_engine/expressions/exact.h"
#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/expressions/false.h"
#include "silo/query_engine/expressions/float_between.h"
#include "silo/query_engine/expressions/float_equals.h"
#include "silo/query_engine/expressions/has_mutation.h"
#include "silo/query_engine/expressions/insertion_contains.h"
#include "silo/query_engine/expressions/int_between.h"
#include "silo/query_engine/expressions/int_equals.h"
#include "silo/query_engine/expressions/is_null.h"
#include "silo/query_engine/expressions/lineage_filter.h"
#include "silo/query_engine/expressions/literal.h"
#include "silo/query_engine/expressions/maybe.h"
#include "silo/query_engine/expressions/mutation_profile.h"
#include "silo/query_engine/expressions/negation.h"
#include "silo/query_engine/expressions/nof.h"
#include "silo/query_engine/expressions/or.h"
#include "silo/query_engine/expressions/phylo_child_filter.h"
#include "silo/query_engine/expressions/string_equals.h"
#include "silo/query_engine/expressions/string_in_set.h"
#include "silo/query_engine/expressions/string_search.h"
#include "silo/query_engine/expressions/symbol_equals.h"
#include "silo/query_engine/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"
#include "silo/query_engine/order_by_field.h"
#include "silo/query_engine/saneql/ast.h"
#include "silo/query_engine/saneql/function_registry.h"
#include "silo/query_engine/saneql/parser.h"

namespace silo::query_engine::saneql {

namespace {

FilterPtr convertEqualsToFilter(const std::string& column_name, const ast::Expression& value_expr) {
   if (isNullLiteral(value_expr)) {
      return std::make_unique<expressions::IsNull>(column_name);
   }
   if (isStringLiteral(value_expr)) {
      return std::make_unique<expressions::StringEquals>(
         column_name, extractStringLiteral(value_expr)
      );
   }
   if (isIntLiteral(value_expr)) {
      return std::make_unique<expressions::IntEquals>(column_name, extractInt32Literal(value_expr));
   }
   if (isFloatLiteral(value_expr)) {
      return std::make_unique<expressions::FloatEquals>(
         column_name, extractNumericAsFloatLiteral(value_expr)
      );
   }
   if (isBoolLiteral(value_expr)) {
      return std::make_unique<expressions::BoolEquals>(
         column_name, std::get<ast::BoolLiteral>(value_expr.value).value
      );
   }
   if (isDateExpression(value_expr)) {
      return std::make_unique<expressions::DateEquals>(column_name, extractDateValue(value_expr));
   }
   throw IllegalQueryException(
      "unsupported value type in equality at {}:{}",
      value_expr.location.line,
      value_expr.location.column
   );
}

FilterPtr convertIntComparison(
   const std::string& column_name,
   ast::BinaryOp binary_op,
   uint32_t value
) {
   switch (binary_op) {
      case ast::BinaryOp::LESS_THAN:
         throw IllegalQueryException("less than is not implemented for integer expressions");
      case ast::BinaryOp::LESS_EQUAL:
         return std::make_unique<expressions::IntBetween>(column_name, std::nullopt, value);
      case ast::BinaryOp::GREATER_THAN:
         throw IllegalQueryException("greater than is not implemented for integer expressions");
      case ast::BinaryOp::GREATER_EQUAL:
         return std::make_unique<expressions::IntBetween>(column_name, value, std::nullopt);
      default:
         throw IllegalQueryException("unexpected operator for integer comparison");
   }
}

FilterPtr convertFloatComparison(
   const std::string& column_name,
   ast::BinaryOp binary_op,
   double value
) {
   switch (binary_op) {
      case ast::BinaryOp::LESS_THAN:
         return std::make_unique<expressions::FloatBetween>(column_name, std::nullopt, value);
      case ast::BinaryOp::LESS_EQUAL:
         throw IllegalQueryException("less equal is not implemented for float expressions");
      case ast::BinaryOp::GREATER_THAN:
         throw IllegalQueryException("greater than is not implemented for float expressions");
      case ast::BinaryOp::GREATER_EQUAL:
         return std::make_unique<expressions::FloatBetween>(column_name, value, std::nullopt);
      default:
         throw IllegalQueryException("unexpected operator for float comparison");
   }
}

FilterPtr convertDateComparison(
   const std::string& column_name,
   ast::BinaryOp binary_op,
   const ast::Expression& value_expr
) {
   auto date_val = extractOptionalDateValue(value_expr);
   switch (binary_op) {
      case ast::BinaryOp::LESS_THAN:
         throw IllegalQueryException("less than is not implemented for date expressions");
      case ast::BinaryOp::LESS_EQUAL:
         return std::make_unique<expressions::DateBetween>(column_name, std::nullopt, date_val);
      case ast::BinaryOp::GREATER_THAN:
         throw IllegalQueryException("greater than is not implemented for date expressions");
      case ast::BinaryOp::GREATER_EQUAL:
         return std::make_unique<expressions::DateBetween>(column_name, date_val, std::nullopt);
      default:
         throw IllegalQueryException("unexpected operator for date comparison");
   }
}

FilterPtr convertComparisonToFilter(
   const std::string& column_name,
   ast::BinaryOp binary_op,
   const ast::Expression& value_expr
) {
   if (isDateExpression(value_expr)) {
      return convertDateComparison(column_name, binary_op, value_expr);
   }
   if (isFloatLiteral(value_expr)) {
      return convertFloatComparison(
         column_name, binary_op, extractNumericAsFloatLiteral(value_expr)
      );
   }
   if (isIntLiteral(value_expr)) {
      return convertIntComparison(column_name, binary_op, extractInt32Literal(value_expr));
   }
   throw IllegalQueryException(
      "unsupported value type in comparison at {}:{}",
      value_expr.location.line,
      value_expr.location.column
   );
}

FilterPtr convertBinaryExprToFilter(const ast::BinaryExpr& bin_expr) {
   switch (bin_expr.op) {
      case ast::BinaryOp::AND: {
         expressions::ExpressionVector children;
         children.push_back(convertToFilter(*bin_expr.left));
         children.push_back(convertToFilter(*bin_expr.right));
         return std::make_unique<expressions::And>(std::move(children));
      }
      case ast::BinaryOp::OR: {
         expressions::ExpressionVector children;
         children.push_back(convertToFilter(*bin_expr.left));
         children.push_back(convertToFilter(*bin_expr.right));
         return std::make_unique<expressions::Or>(std::move(children));
      }
      case ast::BinaryOp::EQUALS: {
         if (std::holds_alternative<ast::Identifier>(bin_expr.left->value)) {
            return convertEqualsToFilter(extractIdentifierName(*bin_expr.left), *bin_expr.right);
         }
         if (std::holds_alternative<ast::Identifier>(bin_expr.right->value)) {
            return convertEqualsToFilter(extractIdentifierName(*bin_expr.right), *bin_expr.left);
         }
         throw IllegalQueryException(
            "equality comparison requires an identifier on one side at {}:{}",
            bin_expr.left->location.line,
            bin_expr.left->location.column
         );
      }
      case ast::BinaryOp::NOT_EQUALS: {
         if (std::holds_alternative<ast::Identifier>(bin_expr.left->value)) {
            return std::make_unique<expressions::Negation>(
               convertEqualsToFilter(extractIdentifierName(*bin_expr.left), *bin_expr.right)
            );
         }
         if (std::holds_alternative<ast::Identifier>(bin_expr.right->value)) {
            return std::make_unique<expressions::Negation>(
               convertEqualsToFilter(extractIdentifierName(*bin_expr.right), *bin_expr.left)
            );
         }
         throw IllegalQueryException(
            "not-equals comparison requires an identifier on one side at {}:{}",
            bin_expr.left->location.line,
            bin_expr.left->location.column
         );
      }
      case ast::BinaryOp::LESS_THAN:
      case ast::BinaryOp::LESS_EQUAL:
      case ast::BinaryOp::GREATER_THAN:
      case ast::BinaryOp::GREATER_EQUAL: {
         CHECK_SILO_QUERY(
            std::holds_alternative<ast::Identifier>(bin_expr.left->value),
            "comparison requires an identifier on the left side at {}:{}",
            bin_expr.left->location.line,
            bin_expr.left->location.column
         );
         return convertComparisonToFilter(
            extractIdentifierName(*bin_expr.left), bin_expr.op, *bin_expr.right
         );
      }
   }
   throw IllegalQueryException("unhandled binary operator");
}

// ========================================================================
// Scalar function handlers (registered in ScalarFunctionRegistry)
// ========================================================================

FilterPtr handleBetween(const BoundArguments& args) {
   auto column_name = extractIdentifierName(args.at("column"));
   const auto& from_expr = args.at("from");
   const auto& to_expr = args.at("to");

   if (isDateExpression(from_expr) || isDateExpression(to_expr)) {
      return std::make_unique<expressions::DateBetween>(
         column_name, extractOptionalDateValue(from_expr), extractOptionalDateValue(to_expr)
      );
   }
   if (isFloatLiteral(from_expr) || isFloatLiteral(to_expr)) {
      std::optional<double> from_val;
      std::optional<double> to_val;
      if (!isNullLiteral(from_expr)) {
         from_val = extractNumericAsFloatLiteral(from_expr);
      }
      if (!isNullLiteral(to_expr)) {
         to_val = extractNumericAsFloatLiteral(to_expr);
      }
      return std::make_unique<expressions::FloatBetween>(column_name, from_val, to_val);
   }
   if (isIntLiteral(from_expr) || isIntLiteral(to_expr)) {
      std::optional<int32_t> from_val;
      std::optional<int32_t> to_val;
      if (!isNullLiteral(from_expr)) {
         from_val = extractInt32Literal(from_expr);
      }
      if (!isNullLiteral(to_expr)) {
         to_val = extractInt32Literal(to_expr);
      }
      return std::make_unique<expressions::IntBetween>(column_name, from_val, to_val);
   }
   throw IllegalQueryException(
      "Could not infer type of between expression. From-value or to-value needs to be a typed "
      "non-null value, got from: '{}' to: '{}'",
      from_expr.toString(),
      to_expr.toString()
   );
}

FilterPtr handleIn(const BoundArguments& args) {
   auto column_name = extractIdentifierName(args.at("column"));
   const auto& set_expr = args.at("values");
   CHECK_SILO_QUERY(
      std::holds_alternative<ast::SetLiteral>(set_expr.value),
      "in() expects a set literal argument at {}:{}",
      set_expr.location.line,
      set_expr.location.column
   );
   const auto& set = std::get<ast::SetLiteral>(set_expr.value);
   std::unordered_set<std::string> values;
   for (const auto& elem : set.elements) {
      values.insert(extractStringLiteral(*elem));
   }
   return std::make_unique<expressions::StringInSet>(column_name, std::move(values));
}

FilterPtr handleIsNull(const BoundArguments& args) {
   return std::make_unique<expressions::IsNull>(extractIdentifierName(args.at("column")));
}

FilterPtr handleIsNotNull(const BoundArguments& args) {
   return std::make_unique<expressions::Negation>(
      std::make_unique<expressions::IsNull>(extractIdentifierName(args.at("column")))
   );
}

FilterPtr handleLineage(const BoundArguments& args) {
   auto column_name = extractIdentifierName(args.at("column"));
   const auto& value_expr = args.at("value");
   std::optional<std::string> lineage_value;
   if (!isNullLiteral(value_expr)) {
      lineage_value = extractStringLiteral(value_expr);
   }
   bool include_sublineages = false;
   if (const auto* expr = args.get("includeSublineages")) {
      include_sublineages = extractBoolLiteral(*expr);
   }
   std::optional<common::RecombinantEdgeFollowingMode> sublineage_mode;
   if (include_sublineages) {
      sublineage_mode = common::RecombinantEdgeFollowingMode::DO_NOT_FOLLOW;
   }
   auto recombinant_mode = args.getOptionalString("recombinantFollowingMode");
   if (recombinant_mode.has_value()) {
      if (recombinant_mode.value() == "alwaysFollow") {
         sublineage_mode = common::RecombinantEdgeFollowingMode::ALWAYS_FOLLOW;
      } else if (recombinant_mode.value() == "followIfFullyContainedInClade") {
         sublineage_mode = common::RecombinantEdgeFollowingMode::FOLLOW_IF_FULLY_CONTAINED_IN_CLADE;
      } else if (recombinant_mode.value() == "doNotFollow") {
         sublineage_mode = common::RecombinantEdgeFollowingMode::DO_NOT_FOLLOW;
      } else {
         throw IllegalQueryException(
            "invalid recombinantFollowingMode: '{}'. Valid values are: alwaysFollow, "
            "followIfFullyContainedInClade, doNotFollow",
            recombinant_mode.value()
         );
      }
   }
   return std::make_unique<expressions::LineageFilter>(column_name, lineage_value, sublineage_mode);
}

FilterPtr handlePhyloDescendantOf(const BoundArguments& args) {
   return std::make_unique<expressions::PhyloChildFilter>(
      extractIdentifierName(args.at("column")), extractStringLiteral(args.at("node"))
   );
}

FilterPtr handleLike(const BoundArguments& args) {
   auto column_name = extractIdentifierName(args.at("column"));
   auto pattern = extractStringLiteral(args.at("pattern"));
   auto regex = std::make_unique<re2::RE2>(pattern);
   CHECK_SILO_QUERY(
      regex->ok(),
      "Invalid Regular Expression. The parsing of the regular expression failed with the "
      "error '{}'. See https://github.com/google/re2/wiki/Syntax for a Syntax specification.",
      regex->error()
   );
   return std::make_unique<expressions::StringSearch>(column_name, std::move(regex));
}

template <typename SymbolType>
FilterPtr handleSymbolEquals(const BoundArguments& args) {
   const uint32_t position = extractUint32Literal(args.at("position"));
   CHECK_SILO_QUERY(position > 0, "The field 'position' is 1-indexed. Value of 0 not allowed.");
   const uint32_t position_idx = position - 1;
   auto symbol_str = extractStringLiteral(args.at("symbol"));
   CHECK_SILO_QUERY(
      symbol_str.size() == 1, "{}() symbol must be a single character", args.functionName()
   );
   auto sequence_name = args.getOptionalString("sequenceName");
   char symbol_char = symbol_str[0];
   if (symbol_char == '.') {
      return std::make_unique<expressions::SymbolEquals<SymbolType>>(
         sequence_name, position_idx, expressions::SymbolOrDot<SymbolType>::dot()
      );
   }
   auto symbol = SymbolType::charToSymbol(symbol_char);
   CHECK_SILO_QUERY(
      symbol.has_value(), "{}() invalid symbol '{}'", args.functionName(), symbol_char
   );
   return std::make_unique<expressions::SymbolEquals<SymbolType>>(
      sequence_name, position_idx, expressions::SymbolOrDot<SymbolType>(symbol.value())
   );
}

template <typename SymbolType>
FilterPtr handleHasMutation(const BoundArguments& args) {
   const uint32_t position = extractUint32Literal(args.at("position"));
   CHECK_SILO_QUERY(position > 0, "The field 'position' is 1-indexed. Value of 0 not allowed.");
   auto sequence_name = args.getOptionalString("sequenceName");
   return std::make_unique<expressions::HasMutation<SymbolType>>(
      std::move(sequence_name), position - 1
   );
}

template <typename SymbolType>
FilterPtr handleInsertionContains(const BoundArguments& args) {
   auto position = extractUint32Literal(args.at("position"));
   auto value = extractStringLiteral(args.at("value"));
   CHECK_SILO_QUERY(
      !value.empty(),
      "The field 'value' in an InsertionContains expression must not be an empty string"
   );
   auto sequence_name = args.getOptionalString("sequenceName");
   return std::make_unique<expressions::InsertionContains<SymbolType>>(
      sequence_name, position, std::move(value)
   );
}

FilterPtr handleExact(const BoundArguments& args) {
   return std::make_unique<expressions::Exact>(convertToFilter(args.at("child")));
}

FilterPtr handleMaybe(const BoundArguments& args) {
   return std::make_unique<expressions::Maybe>(convertToFilter(args.at("child")));
}

// NOLINTNEXTLINE(misc-no-recursion)
FilterPtr handleNOf(const BoundArguments& args) {
   const int32_t number_of_matchers = extractInt32Literal(args.at("count"));
   bool match_exactly = false;
   if (const auto* expr = args.get("matchExactly")) {
      match_exactly = extractBoolLiteral(*expr);
   }
   const auto& children_set = extractSetLiteral(args.at("children"));
   expressions::ExpressionVector children;
   for (const auto& child_expr : children_set.elements) {
      children.push_back(convertToFilter(*child_expr));
   }
   return std::make_unique<expressions::NOf>(
      std::move(children), number_of_matchers, match_exactly
   );
}

template <typename SymbolType>
expressions::MutationProfile<SymbolType>::Mutation parseMutationRecord(
   const ast::RecordLiteral& record
) {
   uint32_t position_idx = 0;
   bool found_position = false;
   std::string symbol_str;
   bool found_symbol = false;

   for (const auto& field : record.fields) {
      if (field.name == "position") {
         const uint32_t pos_val = extractUint32Literal(*field.value);
         CHECK_SILO_QUERY(
            pos_val > 0,
            "The 'position' field in a {} MutationProfile mutation is 1-indexed; "
            "value 0 is not allowed",
            SymbolType::SYMBOL_NAME
         );
         position_idx = pos_val - 1;
         found_position = true;
      } else if (field.name == "symbol") {
         symbol_str = extractStringLiteral(*field.value);
         found_symbol = true;
      }
   }

   CHECK_SILO_QUERY(
      found_position,
      "Each mutation in a {} MutationProfile expression must have a 'position' field",
      SymbolType::SYMBOL_NAME
   );
   CHECK_SILO_QUERY(
      found_symbol,
      "Each mutation in a {} MutationProfile expression must have a 'symbol' field",
      SymbolType::SYMBOL_NAME
   );
   CHECK_SILO_QUERY(
      symbol_str.size() == 1,
      "The 'symbol' field in a {} MutationProfile mutation must be a single character",
      SymbolType::SYMBOL_NAME
   );
   const auto sym = SymbolType::charToSymbol(symbol_str[0]);
   CHECK_SILO_QUERY(
      sym.has_value(),
      "Invalid {} symbol '{}' in MutationProfile",
      SymbolType::SYMBOL_NAME,
      symbol_str[0]
   );

   return {position_idx, sym.value()};
}

template <typename SymbolType>
std::vector<typename expressions::MutationProfile<SymbolType>::Mutation> parseMutationList(
   const ast::SetLiteral& mutations_set
) {
   using MP = expressions::MutationProfile<SymbolType>;
   std::vector<typename MP::Mutation> parsed_mutations;
   for (const auto& elem : mutations_set.elements) {
      CHECK_SILO_QUERY(
         std::holds_alternative<ast::RecordLiteral>(elem->value),
         "Each element of 'mutations' in a {} MutationProfile expression must be a record "
         "literal with 'position' and 'symbol' fields",
         SymbolType::SYMBOL_NAME
      );
      parsed_mutations.push_back(
         parseMutationRecord<SymbolType>(std::get<ast::RecordLiteral>(elem->value))
      );
   }
   return parsed_mutations;
}

template <typename SymbolType>
FilterPtr handleMutationProfile(const BoundArguments& args) {
   const uint32_t distance = extractUint32Literal(args.at("distance"));
   auto sequence_name = args.getOptionalString("sequenceName");

   const auto* query_seq_expr = args.get("querySequence");
   const auto* sequence_id_expr = args.get("sequenceId");
   const auto* mutations_expr = args.get("mutations");
   const int input_count = static_cast<int>(query_seq_expr != nullptr) +
                           static_cast<int>(sequence_id_expr != nullptr) +
                           static_cast<int>(mutations_expr != nullptr);
   CHECK_SILO_QUERY(
      input_count == 1,
      "Exactly one of 'querySequence', 'sequenceId', or 'mutations' must be provided in a {} "
      "MutationProfile expression, but {} were provided",
      SymbolType::SYMBOL_NAME,
      input_count
   );

   using MP = expressions::MutationProfile<SymbolType>;

   if (query_seq_expr != nullptr) {
      return std::make_unique<MP>(
         sequence_name,
         distance,
         typename MP::QuerySequenceInput{extractStringLiteral(*query_seq_expr)}
      );
   }
   if (sequence_id_expr != nullptr) {
      return std::make_unique<MP>(
         sequence_name,
         distance,
         typename MP::SequenceIdInput{extractStringLiteral(*sequence_id_expr)}
      );
   }

   CHECK_SILO_QUERY(
      std::holds_alternative<ast::SetLiteral>(mutations_expr->value),
      "The 'mutations' argument of a {} MutationProfile expression must be a set literal",
      SymbolType::SYMBOL_NAME
   );
   auto parsed_mutations =
      parseMutationList<SymbolType>(std::get<ast::SetLiteral>(mutations_expr->value));
   return std::make_unique<MP>(
      sequence_name, distance, typename MP::MutationsInput{std::move(parsed_mutations)}
   );
}

}  // namespace

std::unique_ptr<expressions::Expression> convertToFilter(const ast::Expression& ast) {
   return std::visit(
      [&](const auto& node) -> FilterPtr {
         using T = std::decay_t<decltype(node)>;

         if constexpr (std::is_same_v<T, ast::BinaryExpr>) {
            return convertBinaryExprToFilter(node);
         } else if constexpr (std::is_same_v<T, ast::UnaryNotExpr>) {
            return std::make_unique<expressions::Negation>(convertToFilter(*node.operand));
         } else if constexpr (std::is_same_v<T, ast::BoolLiteral>) {
            if (node.value) {
               return std::make_unique<expressions::True>();
            }
            return std::make_unique<expressions::False>();
         } else if constexpr (std::is_same_v<T, ast::FunctionCall>) {
            const auto* entry = ScalarFunctionRegistry::instance().findFunction(node.function_name);
            CHECK_SILO_QUERY(entry != nullptr, "unknown scalar function '{}'", node.function_name);
            auto bound = bindArguments(
               node.function_name, entry->signature, node.positional_arguments, node.named_arguments
            );
            return entry->handler(bound);
         } else {
            throw IllegalQueryException(
               "unsupported expression type in filter context at {}:{}",
               ast.location.line,
               ast.location.column
            );
         }
      },
      ast.value
   );
}

// ========================================================================
// Pipeline function handlers (registered in FunctionRegistry)
// ========================================================================

namespace {

operators::AggregateFunction parseAggregateFunctionName(const std::string& function_name) {
   if (function_name == "count") {
      return operators::AggregateFunction::COUNT;
   }
   throw IllegalQueryException(
      "unknown aggregate function '{}'. Valid functions: count", function_name
   );
}

struct GroupByArgs {
   std::vector<schema::ColumnIdentifier> group_by_fields;
   std::vector<operators::AggregateDefinition> aggregates;
};

operators::AggregateDefinition parseAggregateDefinition(
   const ast::RecordField& field,
   const std::vector<schema::ColumnIdentifier>& schema
) {
   CHECK_SILO_QUERY(
      std::holds_alternative<ast::FunctionCall>(field.value->value),
      "aggregate definition '{}' must be a function call (e.g. count(), sum(col))",
      field.name
   );
   const auto& func = std::get<ast::FunctionCall>(field.value->value);
   auto agg_func = parseAggregateFunctionName(func.function_name);
   std::optional<schema::ColumnIdentifier> source_column;
   if (!func.positional_arguments.empty()) {
      auto source_column_name = extractIdentifierName(*func.positional_arguments[0].value);
      auto found = std::ranges::find_if(schema, [&](const auto& col) {
         return col.name == source_column_name;
      });
      CHECK_SILO_QUERY(
         found != schema.end(),
         "source column {} is not present in the input's output schema",
         source_column_name
      );
      source_column = *found;
   }
   return {
      .output_name = field.name, .function = agg_func, .source_column = std::move(source_column)
   };
}

std::vector<schema::ColumnIdentifier> parseGroupByFields(
   const ast::SetLiteral& set,
   const std::vector<schema::ColumnIdentifier>& schema
) {
   std::vector<schema::ColumnIdentifier> group_by_fields;
   for (const auto& elem : set.elements) {
      auto group_by_name = extractIdentifierName(*elem);
      auto found =
         std::ranges::find_if(schema, [&](const auto& col) { return col.name == group_by_name; });
      CHECK_SILO_QUERY(
         found != schema.end(),
         "groupBy field '{}' is not present in the input's output schema",
         group_by_name
      );
      group_by_fields.push_back(*found);
   }
   return group_by_fields;
}

GroupByArgs parseGroupBySpecs(
   const BoundArguments& args,
   const std::vector<schema::ColumnIdentifier>& child_schema
) {
   GroupByArgs result;

   // Parse aggregates (required) — a RecordLiteral like {count:=count()}
   const auto& agg_expr = args.at("aggregates");
   CHECK_SILO_QUERY(
      std::holds_alternative<ast::RecordLiteral>(agg_expr.value),
      "groupBy aggregates must be a record literal like {{count:=count()}}"
   );
   const auto& record = std::get<ast::RecordLiteral>(agg_expr.value);
   for (const auto& field : record.fields) {
      result.aggregates.push_back(parseAggregateDefinition(field, child_schema));
   }
   // Parse columns (optional) — a SetLiteral like {pango_lineage, division}
   if (const auto* columns_expr = args.get("columns")) {
      const auto& set = extractSetLiteral(*columns_expr);
      result.group_by_fields = parseGroupByFields(set, child_schema);
   }

   return result;
}

std::vector<std::string> names(const std::vector<schema::ColumnIdentifier>& schema) {
   std::vector<std::string> names;
   names.reserve(schema.size());
   for (const auto& col : schema) {
      names.push_back(col.name);
   }
   return names;
}

OrderByField parseOrderByField(
   const ast::Expression& expression,
   const std::vector<schema::ColumnIdentifier>& child_schema
) {
   if (std::holds_alternative<ast::Identifier>(expression.value)) {
      const auto identifier_name = extractIdentifierName(expression);
      auto found =
         std::ranges::find_if(child_schema.begin(), child_schema.end(), [&](const auto& col) {
            return col.name == identifier_name;
         });
      CHECK_SILO_QUERY(
         found != child_schema.end(),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         identifier_name,
         fmt::join(names(child_schema), ", ")
      );
      return {.field = *found, .ascending = true};
   }
   if (std::holds_alternative<ast::FunctionCall>(expression.value)) {
      const auto& call = std::get<ast::FunctionCall>(expression.value);
      CHECK_SILO_QUERY(
         call.function_name == "asc" || call.function_name == "desc",
         "orderBy field must be an identifier or asc()/desc() call, got '{}' at {}:{}",
         call.function_name,
         expression.location.line,
         expression.location.column
      );
      CHECK_SILO_QUERY(
         call.positional_arguments.size() == 1 && call.named_arguments.empty(),
         "{}() expects exactly one argument",
         call.function_name
      );
      const auto identifier_name = extractIdentifierName(*call.positional_arguments[0].value);
      auto found =
         std::ranges::find_if(child_schema.begin(), child_schema.end(), [&](const auto& col) {
            return col.name == identifier_name;
         });
      CHECK_SILO_QUERY(
         found != child_schema.end(),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         identifier_name,
         fmt::join(names(child_schema), ", ")
      );
      return {.field = *found, .ascending = call.function_name == "asc"};
   }
   throw IllegalQueryException(
      "orderBy field must be an identifier or asc()/desc() call at {}:{}",
      expression.location.line,
      expression.location.column
   );
}

std::vector<OrderByField> parseOrderByFields(
   const ast::Expression& expression,
   const std::vector<schema::ColumnIdentifier>& child_schema
) {
   const auto& set = extractSetLiteral(expression);
   std::vector<OrderByField> fields;
   for (const auto& elem : set.elements) {
      auto order_by_field = parseOrderByField(*elem, child_schema);
      fields.push_back(order_by_field);
   }
   return fields;
}

namespace {

operators::QueryNodePtr wrapWithDecompressIfNeeded(
   operators::QueryNodePtr node,
   const std::shared_ptr<schema::TableSchema>& table_schema
) {
   std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>> table_schemas;
   for (const auto& col : node->getOutputSchema()) {
      if (schema::isSequenceColumn(col.type)) {
         table_schemas.emplace(col, table_schema);
      }
   }
   auto mapping = operators::buildDecompressColumnMapping(node->getOutputSchema(), table_schemas);
   if (mapping.empty()) {
      return node;
   }
   return std::make_unique<operators::ZstdDecompressNode>(std::move(node), std::move(mapping));
}

}  // namespace

operators::QueryNodePtr buildScanNode(
   const ast::Expression& ast,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   const auto& name = std::get<ast::Identifier>(ast.value).name;
   auto table_name = schema::TableName(name);
   auto iter = tables.find(table_name);
   CHECK_SILO_QUERY(iter != tables.end(), "table '{}' not found in database", table_name.getName());
   const auto table_schema = iter->second->schema;
   std::vector<schema::ColumnIdentifier> fields = iter->second->schema->getColumnIdentifiers();
   auto table_scan = std::make_unique<operators::TableScanNode>(
      iter->second, std::make_unique<expressions::True>(), std::move(fields)
   );
   return wrapWithDecompressIfNeeded(std::move(table_scan), table_schema);
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleFilter(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   auto filter_expr = convertToFilter(args.at("predicate"));
   return std::make_unique<operators::FilterNode>(std::move(child), std::move(filter_expr));
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleGroupBy(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   auto child_schema = child->getOutputSchema();

   auto [group_by_fields, aggregates] = parseGroupBySpecs(args, child_schema);

   return std::make_unique<operators::AggregateNode>(
      std::move(child), std::move(group_by_fields), std::move(aggregates)
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleProject(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   const auto& field_argument = args.at("fields");
   const std::vector<std::string> field_names =
      holds_alternative<ast::Identifier>(field_argument.value)
         ? std::vector{extractIdentifierName(field_argument)}
         : extractSetOfIdentifiers(field_argument);
   auto child = convert_child(args.at("input"), tables);
   auto child_schema = child->getOutputSchema();
   std::vector<schema::ColumnIdentifier> fields;
   for (const auto& name : field_names) {
      auto found =
         std::ranges::find_if(child_schema, [&](const auto& col) { return col.name == name; });
      CHECK_SILO_QUERY(
         found != child_schema.end(),
         "project field '{}' is not present in the input's output schema",
         name
      );
      fields.emplace_back(name, found->type);
   }
   return std::make_unique<operators::ProjectNode>(std::move(child), std::move(fields));
}

namespace {

using operators::MapNode;

/// Parses a single `name := value` assignment of a map() record. For now only
/// literals (int, float, string, bool) are supported as the assigned value.
MapNode::Assignment parseMapAssignment(const ast::RecordField& field) {
   const auto& [value, location] = *field.value;

   if (std::holds_alternative<ast::IntLiteral>(value)) {
      const int64_t int_value = std::get<ast::IntLiteral>(value).value;
      return {
         .output_column = {.name = field.name, .type = schema::ColumnType::INT64},
         .expression = std::make_unique<expressions::Int64Literal>(int_value)
      };
   }
   if (std::holds_alternative<ast::FloatLiteral>(value)) {
      const double float_value = std::get<ast::FloatLiteral>(value).value;
      return {
         .output_column = {.name = field.name, .type = schema::ColumnType::FLOAT},
         .expression = std::make_unique<expressions::FloatLiteral>(float_value)
      };
   }
   if (std::holds_alternative<ast::StringLiteral>(value)) {
      std::string string_value = std::get<ast::StringLiteral>(value).value;
      return {
         .output_column = {.name = field.name, .type = schema::ColumnType::STRING},
         .expression = std::make_unique<expressions::StringLiteral>(std::move(string_value))
      };
   }
   if (std::holds_alternative<ast::BoolLiteral>(value)) {
      const bool bool_value = std::get<ast::BoolLiteral>(value).value;
      return {
         .output_column = {.name = field.name, .type = schema::ColumnType::BOOL},
         .expression = std::make_unique<expressions::BoolLiteral>(bool_value)
      };
   }

   throw IllegalQueryException(
      "map() field '{}' must be assigned a literal value (int, float, string, or bool) at {}:{}",
      field.name,
      location.line,
      location.column
   );
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleMap(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   const auto& expressions_argument = args.at("expressions");
   CHECK_SILO_QUERY(
      std::holds_alternative<ast::RecordLiteral>(expressions_argument.value),
      "map() expects a record of assignments like {x := 3, y := age}"
   );
   const auto& record = std::get<ast::RecordLiteral>(expressions_argument.value);
   CHECK_SILO_QUERY(!record.fields.empty(), "map() requires at least one assignment");

   std::vector<operators::MapNode::Assignment> assignments;
   assignments.reserve(record.fields.size());
   for (const auto& field : record.fields) {
      assignments.push_back(parseMapAssignment(field));
   }

   auto child = convert_child(args.at("input"), tables);
   return std::make_unique<operators::MapNode>(std::move(child), std::move(assignments));
}

namespace {
constexpr std::string_view NUCLEOTIDE_MUTATIONS_FUNCTION_NAME = "mutations";
constexpr std::string_view AMINO_ACID_MUTATIONS_FUNCTION_NAME = "aminoAcidMutations";
}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleMutations(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   std::vector<std::string> sequence_names;
   if (const auto* seq_expr = args.get("sequenceNames")) {
      sequence_names = extractSetOfIdentifiers(*seq_expr);
   }
   auto min_proportion = extractNumericAsFloatLiteral(args.at("minProportion"));
   CHECK_SILO_QUERY(
      min_proportion >= 0 && min_proportion <= 1,
      "Invalid proportion: minProportion must be in interval [0.0, 1.0]"
   );
   std::vector<std::string> field_strings;
   if (const auto* expr = args.get("fields")) {
      field_strings = extractSetOfIdentifiers(*expr);
   }
   if (args.functionName() == NUCLEOTIDE_MUTATIONS_FUNCTION_NAME) {
      return std::make_unique<operators::UnresolvedMutationsNode<Nucleotide>>(
         std::move(child), std::move(sequence_names), min_proportion, std::move(field_strings)
      );
   }
   return std::make_unique<operators::UnresolvedMutationsNode<AminoAcid>>(
      std::move(child), std::move(sequence_names), min_proportion, std::move(field_strings)
   );
}

namespace {
constexpr std::string_view NUCLEOTIDE_INSERTIONS_FUNCTION_NAME = "insertions";
constexpr std::string_view AMINO_ACID_INSERTIONS_FUNCTION_NAME = "aminoAcidInsertions";
}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleInsertions(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   std::vector<std::string> sequence_names;
   if (const auto* seq_expr = args.get("sequenceNames")) {
      sequence_names = extractSetOfIdentifiers(*seq_expr);
   }
   if (args.functionName() == NUCLEOTIDE_INSERTIONS_FUNCTION_NAME) {
      return std::make_unique<operators::UnresolvedInsertionsNode<Nucleotide>>(
         std::move(child), std::move(sequence_names)
      );
   }
   return std::make_unique<operators::UnresolvedInsertionsNode<AminoAcid>>(
      std::move(child), std::move(sequence_names)
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleRandomize(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   uint32_t seed;
   auto seed_arg = args.getOptionalUint32("seed");
   if (seed_arg.has_value()) {
      seed = seed_arg.value();
   } else {
      seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
   }
   return std::make_unique<operators::OrderByNode>(
      std::move(child), std::vector<OrderByField>{}, seed
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleLimit(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   const uint32_t limit_val = extractUint32Literal(args.at("count"));
   CHECK_SILO_QUERY(limit_val > 0, "limit must be a positive number");
   auto offset = args.getOptionalUint32("offset");
   return std::make_unique<operators::FetchNode>(std::move(child), limit_val, offset);
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleOffset(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   uint32_t offset_val = extractUint32Literal(args.at("count"));
   return std::make_unique<operators::FetchNode>(std::move(child), std::nullopt, offset_val);
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleOrderBy(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto child = convert_child(args.at("input"), tables);
   auto order_fields = parseOrderByFields(args.at("fields"), child->getOutputSchema());
   return std::make_unique<operators::OrderByNode>(
      std::move(child), std::move(order_fields), std::nullopt
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handleMostRecentCommonAncestor(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto column_name = extractStringLiteral(args.at("column"));
   bool print_nodes_not_in_tree = false;
   if (const auto* expr = args.get("printNodesNotInTree")) {
      print_nodes_not_in_tree = extractBoolLiteral(*expr);
   }
   auto child = convert_child(args.at("input"), tables);
   return std::make_unique<operators::UnresolvedMostRecentCommonAncestorNode>(
      std::move(child), std::move(column_name), print_nodes_not_in_tree
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr handlePhyloSubtree(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
) {
   auto column_name = extractStringLiteral(args.at("column"));
   bool print_nodes_not_in_tree = false;
   bool contract_unary_nodes = true;
   if (const auto* expr = args.get("printNodesNotInTree")) {
      print_nodes_not_in_tree = extractBoolLiteral(*expr);
   }
   if (const auto* expr = args.get("contractUnaryNodes")) {
      contract_unary_nodes = extractBoolLiteral(*expr);
   }
   auto child = convert_child(args.at("input"), tables);
   return std::make_unique<operators::UnresolvedPhyloSubtreeNode>(
      std::move(child), std::move(column_name), print_nodes_not_in_tree, contract_unary_nodes
   );
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr convertExpression(
   const ast::Expression& ast,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   if (std::holds_alternative<ast::Identifier>(ast.value)) {
      return buildScanNode(ast, tables);
   }

   CHECK_SILO_QUERY(
      std::holds_alternative<ast::FunctionCall>(ast.value),
      "expected table reference or function call at {}:{}",
      ast.location.line,
      ast.location.column
   );
   const auto& call = std::get<ast::FunctionCall>(ast.value);

   const auto* entry = FunctionRegistry::instance().findFunction(call.function_name);
   CHECK_SILO_QUERY(
      entry != nullptr,
      "unknown function '{}' at {}:{}",
      call.function_name,
      ast.location.line,
      ast.location.column
   );

   auto bound = bindArguments(
      call.function_name, entry->signature, call.positional_arguments, call.named_arguments
   );
   return entry->handler(bound, tables, convertExpression);
}

// ========================================================================
// Public entry point
// ========================================================================

operators::QueryNodePtr convertToQueryTree(
   const ast::Expression& ast,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   return convertExpression(ast, tables);
}

operators::QueryNodePtr parseAndConvertToQueryTree(
   std::string_view query_string,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   Parser parser(query_string);
   auto ast = parser.parse();
   return convertToQueryTree(*ast, tables);
}

// ========================================================================
// Registry construction — signatures + handlers
// ========================================================================

// Shorthand helpers for building signatures
namespace {
ParameterDefinition pos(std::string name, bool required = true) {
   return {.name = std::move(name), .required = required, .positional = true};
}
ParameterDefinition named(std::string name, bool required = true) {
   return {.name = std::move(name), .required = required, .positional = false};
}
}  // namespace

FunctionRegistry::FunctionRegistry() {
   registerFunction("filter", {{pos("input"), pos("predicate")}}, handleFilter);

   registerFunction(
      "groupBy", {{pos("input"), pos("aggregates"), pos("columns", false)}}, handleGroupBy
   );

   registerFunction("project", {{pos("input"), pos("fields")}}, handleProject);

   registerFunction("map", {{pos("input"), pos("expressions")}}, handleMap);

   auto mutations_sig = FunctionSignature{
      {pos("input"), named("minProportion"), named("sequenceNames", false), named("fields", false)}
   };
   registerFunction(
      std::string{NUCLEOTIDE_MUTATIONS_FUNCTION_NAME}, mutations_sig, handleMutations
   );
   registerFunction(
      std::string{AMINO_ACID_MUTATIONS_FUNCTION_NAME}, mutations_sig, handleMutations
   );

   auto insertions_sig = FunctionSignature{{pos("input"), named("sequenceNames", false)}};
   registerFunction(
      std::string{NUCLEOTIDE_INSERTIONS_FUNCTION_NAME}, insertions_sig, handleInsertions
   );
   registerFunction(
      std::string{AMINO_ACID_INSERTIONS_FUNCTION_NAME}, insertions_sig, handleInsertions
   );

   registerFunction("randomize", {{pos("input"), named("seed", false)}}, handleRandomize);

   registerFunction("limit", {{pos("input"), pos("count")}}, handleLimit);

   registerFunction("offset", {{pos("input"), pos("count")}}, handleOffset);

   registerFunction("orderBy", {{pos("input"), pos("fields")}}, handleOrderBy);

   registerFunction(
      "mostRecentCommonAncestor",
      {{pos("input"), pos("column"), named("printNodesNotInTree", false)}},
      handleMostRecentCommonAncestor
   );

   registerFunction(
      "phyloSubtree",
      {{pos("input"),
        pos("column"),
        named("printNodesNotInTree", false),
        named("contractUnaryNodes", false)}},
      handlePhyloSubtree
   );
}

FunctionRegistry& FunctionRegistry::instance() {
   static FunctionRegistry registry;
   return registry;
}

ScalarFunctionRegistry::ScalarFunctionRegistry() {
   registerFunction("between", {{pos("column"), pos("from"), pos("to")}}, handleBetween);

   registerFunction("in", {{pos("column"), pos("values")}}, handleIn);

   registerFunction("isNull", {{pos("column")}}, handleIsNull);
   registerFunction("isNotNull", {{pos("column")}}, handleIsNotNull);

   registerFunction(
      "lineage",
      {{pos("column"),
        pos("value"),
        named("includeSublineages", false),
        named("recombinantFollowingMode", false)}},
      handleLineage
   );

   registerFunction("phyloDescendantOf", {{pos("column"), pos("node")}}, handlePhyloDescendantOf);

   registerFunction("like", {{pos("column"), pos("pattern")}}, handleLike);

   auto symbol_equals_sig =
      FunctionSignature{{named("position"), named("symbol"), named("sequenceName", false)}};
   registerFunction("nucleotideEquals", symbol_equals_sig, handleSymbolEquals<Nucleotide>);
   registerFunction("aminoAcidEquals", symbol_equals_sig, handleSymbolEquals<AminoAcid>);

   auto has_mutation_sig = FunctionSignature{{named("position"), named("sequenceName", false)}};
   registerFunction("hasMutation", has_mutation_sig, handleHasMutation<Nucleotide>);
   registerFunction("hasAAMutation", has_mutation_sig, handleHasMutation<AminoAcid>);

   auto insertion_contains_sig =
      FunctionSignature{{named("position"), named("value"), named("sequenceName", false)}};
   registerFunction("insertionContains", insertion_contains_sig, handleInsertionContains<Nucleotide>);
   registerFunction("aminoAcidInsertionContains", insertion_contains_sig, handleInsertionContains<AminoAcid>);

   registerFunction("exact", {{pos("child")}}, handleExact);
   registerFunction("maybe", {{pos("child")}}, handleMaybe);

   registerFunction(
      "nOf", {{pos("count"), pos("children"), named("matchExactly", false)}}, handleNOf
   );

   auto mutation_profile_sig = FunctionSignature{{
      named("distance"),
      named("sequenceName", false),
      named("querySequence", false),
      named("sequenceId", false),
      named("mutations", false),
   }};
   registerFunction("nucleotideMutationProfile", mutation_profile_sig, handleMutationProfile<Nucleotide>);
   registerFunction("aminoAcidMutationProfile", mutation_profile_sig, handleMutationProfile<AminoAcid>);
}

ScalarFunctionRegistry& ScalarFunctionRegistry::instance() {
   static ScalarFunctionRegistry registry;
   return registry;
}

}  // namespace silo::query_engine::saneql
