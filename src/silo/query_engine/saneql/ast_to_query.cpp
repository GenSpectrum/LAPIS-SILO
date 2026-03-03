#include "silo/query_engine/saneql/ast_to_query.h"

#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "silo/common/aa_symbols.h"
#include "silo/common/date.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/insertions.h"
#include "silo/query_engine/actions/most_recent_common_ancestor.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/actions/phylo_subtree.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/bool_equals.h"
#include "silo/query_engine/filter/expressions/date_between.h"
#include "silo/query_engine/filter/expressions/exact.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/float_between.h"
#include "silo/query_engine/filter/expressions/float_equals.h"
#include "silo/query_engine/filter/expressions/has_mutation.h"
#include "silo/query_engine/filter/expressions/insertion_contains.h"
#include "silo/query_engine/filter/expressions/int_between.h"
#include "silo/query_engine/filter/expressions/int_equals.h"
#include "silo/query_engine/filter/expressions/is_null.h"
#include "silo/query_engine/filter/expressions/lineage_filter.h"
#include "silo/query_engine/filter/expressions/maybe.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/nof.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/phylo_child_filter.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/string_search.h"
#include "silo/query_engine/filter/expressions/symbol_equals.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/saneql/parse_exception.h"

namespace silo::query_engine::saneql {

using filter::expressions::Expression;
using ExprPtr = std::unique_ptr<Expression>;

namespace {

std::optional<std::string> findNamedStringArg(
   const std::vector<ast::Argument>& args,
   const std::string& name
) {
   for (const auto& arg : args) {
      if (arg.name.has_value() && arg.name.value() == name) {
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            return str->value;
         }
         throw IllegalQueryException("error: '{}' field must be a string", name);
      }
   }
   return std::nullopt;
}

std::optional<int64_t> findNamedIntArg(
   const std::vector<ast::Argument>& args,
   const std::string& name
) {
   for (const auto& arg : args) {
      if (arg.name.has_value() && arg.name.value() == name) {
         if (auto* val = std::get_if<ast::IntLiteral>(&arg.value->value)) {
            return val->value;
         }
         throw IllegalQueryException("error: '{}' field must be an integer", name);
      }
   }
   return std::nullopt;
}

/// Check if a named argument exists with a given name, returning the AST expression if found.
/// Does NOT throw on type mismatch — use for polymorphic args that accept multiple types.
const ast::Expression* findNamedArg(
   const std::vector<ast::Argument>& args,
   const std::string& name
) {
   for (const auto& arg : args) {
      if (arg.name.has_value() && arg.name.value() == name) {
         return arg.value.get();
      }
   }
   return nullptr;
}

std::optional<bool> findNamedBoolArg(
   const std::vector<ast::Argument>& args,
   const std::string& name
) {
   for (const auto& arg : args) {
      if (arg.name.has_value() && arg.name.value() == name) {
         if (auto* val = std::get_if<ast::BoolLiteral>(&arg.value->value)) {
            return val->value;
         }
         throw IllegalQueryException("error: '{}' field in action must be a boolean", name);
      }
   }
   return std::nullopt;
}

std::vector<std::string> findNamedStringSetArg(
   const std::vector<ast::Argument>& args,
   const std::string& name
) {
   std::vector<std::string> result;
   for (const auto& arg : args) {
      if (arg.name.has_value() && arg.name.value() == name) {
         if (auto* set = std::get_if<ast::SetLiteral>(&arg.value->value)) {
            for (const auto& elem : set->elements) {
               if (auto* str = std::get_if<ast::StringLiteral>(&elem->value)) {
                  result.push_back(str->value);
               }
            }
         }
      }
   }
   return result;
}

std::string getIdentifierName(const ast::Expression& expr) {
   if (auto* id = std::get_if<ast::Identifier>(&expr.value)) {
      return id->name;
   }
   throw ParseException("Expected identifier", expr.location);
}

bool isTypeCastDate(const ast::Expression& expr) {
   if (auto* cast = std::get_if<ast::TypeCast>(&expr.value)) {
      return cast->target_type == "date";
   }
   return false;
}

std::string getStringFromExpr(const ast::Expression& expr) {
   if (auto* str = std::get_if<ast::StringLiteral>(&expr.value)) {
      return str->value;
   }
   if (auto* cast = std::get_if<ast::TypeCast>(&expr.value)) {
      if (auto* str = std::get_if<ast::StringLiteral>(&cast->operand->value)) {
         return str->value;
      }
   }
   throw ParseException("Expected string literal", expr.location);
}

bool isActionMethod(const std::string& name) {
   return name == "aggregated" || name == "details" || name == "mutations" || name == "fasta" ||
          name == "fastaAligned" || name == "insertions" || name == "aminoAcidInsertions" ||
          name == "aminoAcidMutations" || name == "phyloSubtree" ||
          name == "mostRecentCommonAncestor";
}

template <typename SymbolType>
filter::expressions::SymbolOrDot<SymbolType> parseSymbolOrDot(const std::string& symbol_str) {
   if (symbol_str.size() != 1) {
      throw IllegalQueryException("The string field 'symbol' must be exactly one character long");
   }
   if (symbol_str.at(0) == '.') {
      return filter::expressions::SymbolOrDot<SymbolType>::dot();
   }
   auto symbol_char = SymbolType::charToSymbol(symbol_str.at(0));
   if (!symbol_char.has_value()) {
      throw IllegalQueryException(
         "The string field 'symbol' must be either a valid {} symbol or the '.' symbol.",
         SymbolType::SYMBOL_NAME
      );
   }
   return filter::expressions::SymbolOrDot<SymbolType>{symbol_char.value()};
}

void applyOrderingParams(actions::Action& action, const std::vector<ast::Argument>& arguments) {
   auto limit = findNamedIntArg(arguments, "limit");
   auto offset = findNamedIntArg(arguments, "offset");
   std::optional<uint32_t> randomize_seed;
   if (const auto* randomize_expr = findNamedArg(arguments, "randomize")) {
      if (auto* int_val = std::get_if<ast::IntLiteral>(&randomize_expr->value)) {
         randomize_seed = static_cast<uint32_t>(int_val->value);
      } else if (auto* bool_val = std::get_if<ast::BoolLiteral>(&randomize_expr->value)) {
         if (bool_val->value) {
            randomize_seed =
               static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
         }
      } else {
         throw IllegalQueryException("error: 'randomize' field must be a boolean or an integer");
      }
   }

   if (limit.has_value() && limit.value() <= 0) {
      throw IllegalQueryException("If the action contains a limit, it must be a positive number");
   }

   if (offset.has_value() && offset.value() < 0) {
      throw IllegalQueryException(
         "If the action contains an offset, it must be a non-negative number"
      );
   }

   if (limit.has_value() || offset.has_value() || randomize_seed.has_value()) {
      action.setOrdering(
         {},
         limit.has_value() ? std::optional<uint32_t>(static_cast<uint32_t>(limit.value()))
                           : std::nullopt,
         offset.has_value() ? std::optional<uint32_t>(static_cast<uint32_t>(offset.value()))
                            : std::nullopt,
         randomize_seed
      );
   }
}

bool isSkippableNamedArg(const ast::Argument& arg, const std::vector<std::string>& names) {
   if (!arg.name.has_value()) {
      return false;
   }
   for (const auto& name : names) {
      if (arg.name.value() == name) {
         return true;
      }
   }
   return false;
}

ExprPtr convertEqualsExpr(const ast::BinaryExpr& bin) {
   // Check for column = null → IsNull
   if (std::holds_alternative<ast::NullLiteral>(bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::IsNull>(std::move(col));
   }

   // Check for column = 'string' → StringEquals
   if (auto* str = std::get_if<ast::StringLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::StringEquals>(std::move(col), str->value);
   }

   // Check for column = int → IntEquals
   if (auto* intlit = std::get_if<ast::IntLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::IntEquals>(
         std::move(col), static_cast<uint32_t>(intlit->value)
      );
   }

   // Check for column = float → FloatEquals
   if (auto* fltlit = std::get_if<ast::FloatLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::FloatEquals>(std::move(col), fltlit->value);
   }

   // Check for column = bool → BoolEquals
   if (auto* boollit = std::get_if<ast::BoolLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::BoolEquals>(std::move(col), boollit->value);
   }

   throw ParseException(
      bin.left->location,
      "Unsupported equality comparison: {} = {}",
      bin.left->toString(),
      bin.right->toString()
   );
}

ExprPtr convertNotEqualsExpr(const ast::BinaryExpr& bin) {
   // col <> value  →  Not(col = value)
   ast::BinaryExpr equals_bin{ast::BinaryOp::Equals, nullptr, nullptr};
   // We reuse convertEqualsExpr by constructing a temporary — but since we can't copy
   // unique_ptrs, we just negate the equivalent equals expression inline
   if (std::holds_alternative<ast::NullLiteral>(bin.right->value)) {
      // col <> null → Not(IsNull(col))
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::Negation>(
         std::make_unique<filter::expressions::IsNull>(std::move(col))
      );
   }
   if (auto* str = std::get_if<ast::StringLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::Negation>(
         std::make_unique<filter::expressions::StringEquals>(std::move(col), str->value)
      );
   }
   if (auto* intlit = std::get_if<ast::IntLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::Negation>(
         std::make_unique<filter::expressions::IntEquals>(
            std::move(col), static_cast<uint32_t>(intlit->value)
         )
      );
   }
   if (auto* fltlit = std::get_if<ast::FloatLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::Negation>(
         std::make_unique<filter::expressions::FloatEquals>(std::move(col), fltlit->value)
      );
   }
   if (auto* boollit = std::get_if<ast::BoolLiteral>(&bin.right->value)) {
      std::string col = getIdentifierName(*bin.left);
      return std::make_unique<filter::expressions::BoolEquals>(std::move(col), !boollit->value);
   }
   throw ParseException(
      bin.left->location,
      "Unsupported not-equals comparison: {} <> {}",
      bin.left->toString(),
      bin.right->toString()
   );
}

ExprPtr convertComparisonExpr(const ast::BinaryExpr& bin) {
   std::string col = getIdentifierName(*bin.left);

   // Determine bounds based on operator direction
   // <  → between(nullopt, value-1) or just use the comparison semantics
   // We map comparisons to Between expressions with open/closed bounds:
   //   col < val  → IntBetween(col, nullopt, val-1) for int
   //   col <= val → IntBetween(col, nullopt, val)
   //   col > val  → IntBetween(col, val+1, nullopt)
   //   col >= val → IntBetween(col, val, nullopt)
   // For dates and floats, similar logic applies

   bool is_less = (bin.op == ast::BinaryOp::LessThan || bin.op == ast::BinaryOp::LessEqual);
   bool is_strict = (bin.op == ast::BinaryOp::LessThan || bin.op == ast::BinaryOp::GreaterThan);

   if (auto* intlit = std::get_if<ast::IntLiteral>(&bin.right->value)) {
      auto val = static_cast<uint32_t>(intlit->value);
      std::optional<uint32_t> from;
      std::optional<uint32_t> to;
      if (is_less) {
         to = is_strict ? val - 1 : val;
      } else {
         from = is_strict ? val + 1 : val;
      }
      return std::make_unique<filter::expressions::IntBetween>(std::move(col), from, to);
   }

   if (auto* fltlit = std::get_if<ast::FloatLiteral>(&bin.right->value)) {
      std::optional<double> from;
      std::optional<double> to;
      if (is_less) {
         to = fltlit->value;
      } else {
         from = fltlit->value;
      }
      return std::make_unique<filter::expressions::FloatBetween>(std::move(col), from, to);
   }

   // Date comparison: col < '2020-01-01'::date
   if (isTypeCastDate(*bin.right)) {
      std::string date_str = getStringFromExpr(*bin.right);
      auto date_val = common::stringToDate(date_str);
      std::optional<common::Date> from;
      std::optional<common::Date> to;
      if (is_less) {
         to = is_strict ? date_val - 1 : date_val;
      } else {
         from = is_strict ? date_val + 1 : date_val;
      }
      return std::make_unique<filter::expressions::DateBetween>(std::move(col), from, to);
   }

   throw ParseException(
      bin.left->location,
      "Unsupported comparison: {} {} {}",
      bin.left->toString(),
      ast::binaryOpToString(bin.op),
      bin.right->toString()
   );
}

ExprPtr convertMethodCallToFilter(const ast::MethodCall& call) {
   std::string receiver_name = getIdentifierName(*call.receiver);

   if (call.method_name == "between") {
      if (call.arguments.size() != 2) {
         throw ParseException(call.receiver->location, "between() requires exactly 2 arguments");
      }

      // Check if arguments are date-typed
      if (isTypeCastDate(*call.arguments[0].value) || isTypeCastDate(*call.arguments[1].value)) {
         std::string from_str = getStringFromExpr(*call.arguments[0].value);
         std::string to_str = getStringFromExpr(*call.arguments[1].value);
         auto date_from = common::stringToDate(from_str);
         auto date_to = common::stringToDate(to_str);
         return std::make_unique<filter::expressions::DateBetween>(
            std::move(receiver_name), date_from, date_to
         );
      }

      // Check for int between
      if (auto* from_int = std::get_if<ast::IntLiteral>(&call.arguments[0].value->value)) {
         if (auto* to_int = std::get_if<ast::IntLiteral>(&call.arguments[1].value->value)) {
            return std::make_unique<filter::expressions::IntBetween>(
               std::move(receiver_name),
               static_cast<uint32_t>(from_int->value),
               static_cast<uint32_t>(to_int->value)
            );
         }
      }

      // Check for float between
      if (auto* from_flt = std::get_if<ast::FloatLiteral>(&call.arguments[0].value->value)) {
         if (auto* to_flt = std::get_if<ast::FloatLiteral>(&call.arguments[1].value->value)) {
            return std::make_unique<filter::expressions::FloatBetween>(
               std::move(receiver_name), from_flt->value, to_flt->value
            );
         }
      }

      throw ParseException(call.receiver->location, "Unsupported argument types for between()");
   }

   if (call.method_name == "in") {
      if (call.arguments.size() != 1) {
         throw ParseException(call.receiver->location, "in() requires exactly 1 argument");
      }
      if (auto* set = std::get_if<ast::SetLiteral>(&call.arguments[0].value->value)) {
         std::unordered_set<std::string> values;
         for (const auto& elem : set->elements) {
            if (auto* str = std::get_if<ast::StringLiteral>(&elem->value)) {
               values.insert(str->value);
            } else {
               throw ParseException(elem->location, "Set elements must be string literals");
            }
         }
         return std::make_unique<filter::expressions::StringInSet>(
            std::move(receiver_name), std::move(values)
         );
      }
      throw ParseException(
         call.arguments[0].value->location, "in() argument must be a set literal"
      );
   }

   if (call.method_name == "like") {
      if (call.arguments.size() != 1) {
         throw ParseException(call.receiver->location, "like() requires exactly 1 argument");
      }
      std::string pattern = getStringFromExpr(*call.arguments[0].value);
      auto regex = std::make_unique<re2::RE2>(pattern);
      if (!regex->ok()) {
         throw IllegalQueryException(
            "Invalid Regular Expression. The parsing of the regular expression failed with the "
            "error '{}'. See https://github.com/google/re2/wiki/Syntax for a Syntax "
            "specification.",
            regex->error()
         );
      }
      return std::make_unique<filter::expressions::StringSearch>(
         std::move(receiver_name), std::move(regex)
      );
   }

   if (call.method_name == "isNull") {
      return std::make_unique<filter::expressions::IsNull>(std::move(receiver_name));
   }

   if (call.method_name == "isNotNull") {
      return std::make_unique<filter::expressions::Negation>(
         std::make_unique<filter::expressions::IsNull>(std::move(receiver_name))
      );
   }

   if (call.method_name == "lineage") {
      if (call.arguments.empty()) {
         throw ParseException(call.receiver->location, "lineage() requires at least 1 argument");
      }

      // Check for null lineage value
      std::optional<std::string> lineage_value;
      if (!std::holds_alternative<ast::NullLiteral>(call.arguments[0].value->value)) {
         lineage_value = getStringFromExpr(*call.arguments[0].value);
      }

      // Check for includeSublineages as bool or string
      bool include_sublineages = false;
      if (const auto* incl_expr = findNamedArg(call.arguments, "includeSublineages")) {
         if (auto* bool_val = std::get_if<ast::BoolLiteral>(&incl_expr->value)) {
            include_sublineages = bool_val->value;
         } else if (auto* str_val = std::get_if<ast::StringLiteral>(&incl_expr->value)) {
            include_sublineages = str_val->value == "true";
         } else {
            throw IllegalQueryException(
               "error: 'includeSublineages' field must be a boolean or a string"
            );
         }
      }

      std::optional<common::RecombinantEdgeFollowingMode> sublineage_mode;
      if (include_sublineages) {
         // Default mode when includeSublineages is true
         sublineage_mode = common::RecombinantEdgeFollowingMode::DO_NOT_FOLLOW;

         // Check for recombinantFollowingMode
         auto mode_str = findNamedStringArg(call.arguments, "recombinantFollowingMode");
         if (mode_str.has_value()) {
            static const std::unordered_map<std::string, common::RecombinantEdgeFollowingMode>
               mode_map{
                  {"doNotFollow", common::RecombinantEdgeFollowingMode::DO_NOT_FOLLOW},
                  {"followIfFullyContainedInClade",
                   common::RecombinantEdgeFollowingMode::FOLLOW_IF_FULLY_CONTAINED_IN_CLADE},
                  {"alwaysFollow", common::RecombinantEdgeFollowingMode::ALWAYS_FOLLOW},
               };
            auto it = mode_map.find(mode_str.value());
            if (it == mode_map.end()) {
               throw IllegalQueryException(
                  "Invalid recombinantFollowingMode: {}", mode_str.value()
               );
            }
            sublineage_mode = it->second;
         }
      }

      return std::make_unique<filter::expressions::LineageFilter>(
         std::move(receiver_name), std::move(lineage_value), sublineage_mode
      );
   }

   if (call.method_name == "phyloDescendantOf") {
      if (call.arguments.empty()) {
         throw ParseException(
            call.receiver->location, "phyloDescendantOf() requires at least 1 argument"
         );
      }
      std::string internal_node = getStringFromExpr(*call.arguments[0].value);
      return std::make_unique<filter::expressions::PhyloChildFilter>(
         std::move(receiver_name), std::move(internal_node)
      );
   }

   throw ParseException(call.receiver->location, "Unknown filter method: {}", call.method_name);
}

ExprPtr convertFunctionCallToFilter(const ast::FunctionCall& call) {
   if (call.function_name == "hasMutation" || call.function_name == "hasAAMutation") {
      auto position = findNamedIntArg(call.arguments, "position");
      auto sequence_name = findNamedStringArg(call.arguments, "sequenceName");

      if (!position.has_value()) {
         // Try positional first argument
         if (!call.arguments.empty() && !call.arguments[0].name.has_value()) {
            if (auto* intlit = std::get_if<ast::IntLiteral>(&call.arguments[0].value->value)) {
               position = intlit->value;
            }
         }
      }

      if (!position.has_value()) {
         throw ParseException(
            call.arguments.empty() ? SourceLocation{} : call.arguments[0].location,
            "hasMutation() requires a 'position' argument"
         );
      }

      // Position is 1-indexed in SaneQL, convert to 0-indexed for internal use
      const auto pos_1_indexed = static_cast<uint32_t>(position.value());
      if (pos_1_indexed == 0) {
         throw IllegalQueryException("The field 'position' is 1-indexed. Value of 0 not allowed.");
      }
      const uint32_t pos_0_indexed = pos_1_indexed - 1;

      if (call.function_name == "hasMutation") {
         return std::make_unique<filter::expressions::HasMutation<Nucleotide>>(
            sequence_name, pos_0_indexed
         );
      }
      return std::make_unique<filter::expressions::HasMutation<AminoAcid>>(
         sequence_name, pos_0_indexed
      );
   }

   if (call.function_name == "nucleotideEquals" || call.function_name == "aminoAcidEquals") {
      auto position = findNamedIntArg(call.arguments, "position");
      auto symbol = findNamedStringArg(call.arguments, "symbol");
      auto sequence_name = findNamedStringArg(call.arguments, "sequenceName");

      if (!position.has_value()) {
         if (!call.arguments.empty() && !call.arguments[0].name.has_value()) {
            if (auto* intlit = std::get_if<ast::IntLiteral>(&call.arguments[0].value->value)) {
               position = intlit->value;
            }
         }
      }
      if (!symbol.has_value()) {
         for (const auto& arg : call.arguments) {
            if (!arg.name.has_value()) {
               if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
                  symbol = str->value;
                  break;
               }
            }
         }
      }

      if (!position.has_value() || !symbol.has_value()) {
         throw ParseException(
            SourceLocation{}, "{}() requires 'position' and 'symbol' arguments", call.function_name
         );
      }

      // Position is 1-indexed in the query, convert to 0-indexed
      if (position.value() <= 0) {
         throw IllegalQueryException("The field 'position' is 1-indexed. Value of 0 not allowed.");
      }
      auto position_idx = static_cast<uint32_t>(position.value() - 1);

      if (call.function_name == "nucleotideEquals") {
         auto symbol_or_dot = parseSymbolOrDot<Nucleotide>(symbol.value());
         return std::make_unique<filter::expressions::SymbolEquals<Nucleotide>>(
            sequence_name, position_idx, symbol_or_dot
         );
      }
      auto symbol_or_dot = parseSymbolOrDot<AminoAcid>(symbol.value());
      return std::make_unique<filter::expressions::SymbolEquals<AminoAcid>>(
         sequence_name, position_idx, symbol_or_dot
      );
   }

   if (call.function_name == "insertionContains" ||
       call.function_name == "aminoAcidInsertionContains") {
      auto position = findNamedIntArg(call.arguments, "position");
      auto value = findNamedStringArg(call.arguments, "value");
      auto sequence_name = findNamedStringArg(call.arguments, "sequenceName");

      if (!position.has_value() || !value.has_value()) {
         throw ParseException(
            SourceLocation{}, "{}() requires 'position' and 'value' arguments", call.function_name
         );
      }

      if (value.value().empty()) {
         throw IllegalQueryException(
            "The field 'value' in an InsertionContains expression must not be an empty string"
         );
      }

      if (call.function_name == "insertionContains") {
         return std::make_unique<filter::expressions::InsertionContains<Nucleotide>>(
            sequence_name, static_cast<uint32_t>(position.value()), value.value()
         );
      }
      return std::make_unique<filter::expressions::InsertionContains<AminoAcid>>(
         sequence_name, static_cast<uint32_t>(position.value()), value.value()
      );
   }

   if (call.function_name == "maybe") {
      if (call.arguments.size() != 1) {
         throw ParseException(SourceLocation{}, "maybe() requires exactly 1 argument");
      }
      auto child = convertToFilter(*call.arguments[0].value);
      return std::make_unique<filter::expressions::Maybe>(std::move(child));
   }

   if (call.function_name == "exact") {
      if (call.arguments.size() != 1) {
         throw ParseException(SourceLocation{}, "exact() requires exactly 1 argument");
      }
      auto child = convertToFilter(*call.arguments[0].value);
      return std::make_unique<filter::expressions::Exact>(std::move(child));
   }

   if (call.function_name == "nOf") {
      if (call.arguments.size() < 2) {
         throw ParseException(SourceLocation{}, "nOf() requires at least 2 arguments");
      }

      auto* n_lit = std::get_if<ast::IntLiteral>(&call.arguments[0].value->value);
      if (!n_lit) {
         throw ParseException(
            call.arguments[0].location, "First argument to nOf() must be an integer"
         );
      }

      filter::expressions::ExpressionVector children;
      for (size_t i = 1; i < call.arguments.size(); i++) {
         children.push_back(convertToFilter(*call.arguments[i].value));
      }

      return std::make_unique<filter::expressions::NOf>(
         std::move(children), static_cast<int>(n_lit->value), false
      );
   }

   throw ParseException(SourceLocation{}, "Unknown function: {}", call.function_name);
}

}  // namespace

ExprPtr convertToFilter(const ast::Expression& ast_expr) {
   return std::visit(
      [&](const auto& node) -> ExprPtr {
         using T = std::decay_t<decltype(node)>;

         if constexpr (std::is_same_v<T, ast::BinaryExpr>) {
            if (node.op == ast::BinaryOp::And) {
               filter::expressions::ExpressionVector children;
               children.push_back(convertToFilter(*node.left));
               children.push_back(convertToFilter(*node.right));
               return std::make_unique<filter::expressions::And>(std::move(children));
            }
            if (node.op == ast::BinaryOp::Or) {
               filter::expressions::ExpressionVector children;
               children.push_back(convertToFilter(*node.left));
               children.push_back(convertToFilter(*node.right));
               return std::make_unique<filter::expressions::Or>(std::move(children));
            }
            if (node.op == ast::BinaryOp::Equals) {
               return convertEqualsExpr(node);
            }
            if (node.op == ast::BinaryOp::NotEquals) {
               return convertNotEqualsExpr(node);
            }
            if (node.op == ast::BinaryOp::LessThan || node.op == ast::BinaryOp::LessEqual ||
                node.op == ast::BinaryOp::GreaterThan || node.op == ast::BinaryOp::GreaterEqual) {
               return convertComparisonExpr(node);
            }
            throw ParseException(
               ast_expr.location,
               "Unsupported binary operator in filter: {}",
               ast::binaryOpToString(node.op)
            );
         } else if constexpr (std::is_same_v<T, ast::UnaryNotExpr>) {
            return std::make_unique<filter::expressions::Negation>(convertToFilter(*node.operand));
         } else if constexpr (std::is_same_v<T, ast::MethodCall>) {
            return convertMethodCallToFilter(node);
         } else if constexpr (std::is_same_v<T, ast::FunctionCall>) {
            return convertFunctionCallToFilter(node);
         } else if constexpr (std::is_same_v<T, ast::BoolLiteral>) {
            if (node.value) {
               return std::make_unique<filter::expressions::True>();
            }
            return std::make_unique<filter::expressions::False>();
         } else if constexpr (std::is_same_v<T, ast::Identifier>) {
            // A bare identifier used as a filter expression — treat as boolean column check
            return std::make_unique<filter::expressions::BoolEquals>(node.name, true);
         } else {
            throw ParseException(
               ast_expr.location, "Cannot convert expression to filter: {}", ast_expr.toString()
            );
         }
      },
      ast_expr.value
   );
}

std::string_view resolveMutationFieldName(const std::string& name) {
   // Map field name strings to static string_view constants to avoid dangling references.
   // These point to string literals which have static storage duration.
   static const std::unordered_map<std::string, std::string_view> field_map = {
      {"mutation", "mutation"},
      {"mutationFrom", "mutationFrom"},
      {"mutationTo", "mutationTo"},
      {"position", "position"},
      {"sequenceName", "sequenceName"},
      {"proportion", "proportion"},
      {"coverage", "coverage"},
      {"count", "count"},
   };
   auto it = field_map.find(name);
   if (it != field_map.end()) {
      return it->second;
   }
   return {};
}

std::unique_ptr<actions::Action> convertToAction(const ast::MethodCall& method_call) {
   const std::vector<std::string> ordering_params = {"limit", "offset", "randomize"};

   if (method_call.method_name == "aggregated") {
      std::vector<std::string> group_by;
      std::unordered_set<std::string> seen;
      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, ordering_params)) {
            continue;
         }
         std::string name;
         if (auto* id = std::get_if<ast::Identifier>(&arg.value->value)) {
            name = id->name;
         } else if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            name = str->value;
         } else {
            continue;
         }
         if (seen.insert(name).second) {
            group_by.push_back(std::move(name));
         }
      }
      auto action = std::make_unique<actions::Aggregated>(std::move(group_by));
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "details") {
      std::vector<std::string> fields;
      std::unordered_set<std::string> seen;

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, ordering_params)) {
            continue;
         }
         std::string name;
         if (auto* id = std::get_if<ast::Identifier>(&arg.value->value)) {
            name = id->name;
         } else if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            name = str->value;
         } else {
            continue;
         }
         if (seen.insert(name).second) {
            fields.push_back(std::move(name));
         }
      }
      auto action = std::make_unique<actions::Details>(std::move(fields));
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "mutations") {
      std::vector<std::string> sequence_names;

      auto min_prop = findNamedStringArg(method_call.arguments, "minProportion");
      if (!min_prop.has_value()) {
         throw IllegalQueryException(
            "Mutations action must contain the field minProportion of type number with limits "
            "[0.0, 1.0]. Only mutations are returned if the proportion of sequences having this "
            "mutation, is at least minProportion"
         );
      }
      double min_proportion = std::stod(min_prop.value());
      if (min_proportion < 0.0 || min_proportion > 1.0) {
         throw IllegalQueryException(
            "Invalid proportion: minProportion must be in interval [0.0, 1.0]"
         );
      }

      std::vector<std::string> skip_names = ordering_params;
      skip_names.emplace_back("minProportion");
      skip_names.emplace_back("fields");

      auto field_names = findNamedStringSetArg(method_call.arguments, "fields");
      std::vector<std::string_view> fields;
      for (const auto& f : field_names) {
         auto resolved = resolveMutationFieldName(f);
         if (resolved.empty()) {
            throw IllegalQueryException(
               "The attribute 'fields' contains an invalid field '{}'. Valid fields are mutation, "
               "mutationFrom, mutationTo, position, sequenceName, proportion, coverage, count.",
               f
            );
         }
         fields.emplace_back(resolved);
      }

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action = std::make_unique<actions::Mutations<Nucleotide>>(
         std::move(sequence_names), min_proportion, std::move(fields)
      );
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "aminoAcidMutations") {
      std::vector<std::string> sequence_names;

      auto min_prop = findNamedStringArg(method_call.arguments, "minProportion");
      if (!min_prop.has_value()) {
         throw IllegalQueryException(
            "Mutations action must contain the field minProportion of type number with limits "
            "[0.0, 1.0]. Only mutations are returned if the proportion of sequences having this "
            "mutation, is at least minProportion"
         );
      }
      double min_proportion = std::stod(min_prop.value());
      if (min_proportion < 0.0 || min_proportion > 1.0) {
         throw IllegalQueryException(
            "Invalid proportion: minProportion must be in interval [0.0, 1.0]"
         );
      }

      std::vector<std::string> skip_names = ordering_params;
      skip_names.emplace_back("minProportion");
      skip_names.emplace_back("fields");

      auto field_names = findNamedStringSetArg(method_call.arguments, "fields");
      std::vector<std::string_view> fields;
      for (const auto& f : field_names) {
         auto resolved = resolveMutationFieldName(f);
         if (resolved.empty()) {
            throw IllegalQueryException(
               "The attribute 'fields' contains an invalid field '{}'. Valid fields are mutation, "
               "mutationFrom, mutationTo, position, sequenceName, proportion, coverage, count.",
               f
            );
         }
         fields.emplace_back(resolved);
      }

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action = std::make_unique<actions::Mutations<AminoAcid>>(
         std::move(sequence_names), min_proportion, std::move(fields)
      );
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "fasta") {
      std::vector<std::string> sequence_names;
      auto additional_fields = findNamedStringSetArg(method_call.arguments, "additionalFields");

      std::vector<std::string> skip_names = ordering_params;
      skip_names.emplace_back("additionalFields");

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action =
         std::make_unique<actions::Fasta>(std::move(sequence_names), std::move(additional_fields));
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "fastaAligned") {
      std::vector<std::string> sequence_names;
      auto additional_fields = findNamedStringSetArg(method_call.arguments, "additionalFields");

      std::vector<std::string> skip_names = ordering_params;
      skip_names.emplace_back("additionalFields");

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action = std::make_unique<actions::FastaAligned>(
         std::move(sequence_names), std::move(additional_fields)
      );
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "insertions") {
      std::vector<std::string> sequence_names;

      std::vector<std::string> skip_names = ordering_params;

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action =
         std::make_unique<actions::InsertionAggregation<Nucleotide>>(std::move(sequence_names));
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "aminoAcidInsertions") {
      std::vector<std::string> sequence_names;

      std::vector<std::string> skip_names = ordering_params;

      for (const auto& arg : method_call.arguments) {
         if (isSkippableNamedArg(arg, skip_names)) {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      auto action =
         std::make_unique<actions::InsertionAggregation<AminoAcid>>(std::move(sequence_names));
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "phyloSubtree") {
      std::string column_name;
      for (const auto& arg : method_call.arguments) {
         if (!arg.name.has_value()) {
            if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
               column_name = str->value;
               break;
            }
         }
      }
      if (column_name.empty()) {
         throw IllegalQueryException("phyloSubtree() requires a column name argument");
      }
      bool print_nodes =
         findNamedBoolArg(method_call.arguments, "printNodesNotInTree").value_or(false);
      bool contract = findNamedBoolArg(method_call.arguments, "contractUnaryNodes").value_or(false);
      auto action =
         std::make_unique<actions::PhyloSubtree>(std::move(column_name), print_nodes, contract);
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   if (method_call.method_name == "mostRecentCommonAncestor") {
      std::string column_name;
      for (const auto& arg : method_call.arguments) {
         if (!arg.name.has_value()) {
            if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
               column_name = str->value;
               break;
            }
         }
      }
      if (column_name.empty()) {
         throw IllegalQueryException("mostRecentCommonAncestor() requires a column name argument");
      }
      bool print_nodes =
         findNamedBoolArg(method_call.arguments, "printNodesNotInTree").value_or(false);
      auto action =
         std::make_unique<actions::MostRecentCommonAncestor>(std::move(column_name), print_nodes);
      applyOrderingParams(*action, method_call.arguments);
      return action;
   }

   throw IllegalQueryException("Unknown action: {}", method_call.method_name);
}

namespace {

std::vector<actions::OrderByField> parseOrderByArgs(const ast::MethodCall& order_by_call) {
   std::vector<actions::OrderByField> order_by_fields;
   for (const auto& arg : order_by_call.arguments) {
      if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
         // Parse 'field [asc|ascending|desc|descending]'
         std::string val = str->value;
         bool ascending = true;
         auto space_pos = val.rfind(' ');
         if (space_pos != std::string::npos) {
            std::string dir = val.substr(space_pos + 1);
            if (dir == "desc" || dir == "descending") {
               ascending = false;
               val = val.substr(0, space_pos);
            } else if (dir == "asc" || dir == "ascending") {
               val = val.substr(0, space_pos);
            }
         }
         order_by_fields.push_back({std::move(val), ascending});
      } else if (auto* id = std::get_if<ast::Identifier>(&arg.value->value)) {
         order_by_fields.push_back({id->name, true});
      }
   }
   return order_by_fields;
}

}  // namespace

std::shared_ptr<query_engine::Query> convertToQuery(const ast::Expression& ast_expr) {
   // Expected shape: table.filter(...).action(...).orderBy(...)
   // or: table.action(...).orderBy(...)
   // or: table.filter(...).action(...)
   // or: table.action(...)
   if (auto* outer_call = std::get_if<ast::MethodCall>(&ast_expr.value)) {
      // Check if outermost call is orderBy — peel it off
      if (outer_call->method_name == "orderBy") {
         auto order_by_fields = parseOrderByArgs(*outer_call);

         // The receiver should be the action call
         auto query = convertToQuery(*outer_call->receiver);

         // Apply ordering to the action (preserves limit/offset/randomize already set)
         query->action->setOrderByFields(order_by_fields);
         return query;
      }

      // Check if this is an action method
      if (isActionMethod(outer_call->method_name)) {
         auto action = convertToAction(*outer_call);

         // Check if receiver is filter call
         if (auto* filter_call = std::get_if<ast::MethodCall>(&outer_call->receiver->value)) {
            if (filter_call->method_name == "filter") {
               if (filter_call->arguments.size() != 1) {
                  throw ParseException(
                     filter_call->receiver->location, "filter() requires exactly 1 argument"
                  );
               }
               auto filter = convertToFilter(*filter_call->arguments[0].value);
               return std::make_shared<query_engine::Query>(std::move(filter), std::move(action));
            }
         }

         // No filter — use True
         auto filter = std::make_unique<filter::expressions::True>();
         return std::make_shared<query_engine::Query>(std::move(filter), std::move(action));
      }
   }

   throw ParseException(
      ast_expr.location, "Query must end with an action (e.g., .aggregated(), .details())"
   );
}

}  // namespace silo::query_engine::saneql
