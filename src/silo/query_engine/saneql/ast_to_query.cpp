#include "silo/query_engine/saneql/ast_to_query.h"

#include <optional>
#include <string>
#include <unordered_set>

#include "silo/common/aa_symbols.h"
#include "silo/common/date.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/bool_equals.h"
#include "silo/query_engine/filter/expressions/date_between.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/float_between.h"
#include "silo/query_engine/filter/expressions/float_equals.h"
#include "silo/query_engine/filter/expressions/has_mutation.h"
#include "silo/query_engine/filter/expressions/int_between.h"
#include "silo/query_engine/filter/expressions/int_equals.h"
#include "silo/query_engine/filter/expressions/is_null.h"
#include "silo/query_engine/filter/expressions/lineage_filter.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/nof.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/string_search.h"
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
      }
   }
   return std::nullopt;
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

ExprPtr convertMethodCallToFilter(const ast::MethodCall& call) {
   std::string receiver_name = getIdentifierName(*call.receiver);

   if (call.method_name == "between") {
      if (call.arguments.size() != 2) {
         throw ParseException(
            call.receiver->location, "between() requires exactly 2 arguments"
         );
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

      throw ParseException(
         call.receiver->location, "Unsupported argument types for between()"
      );
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
      return std::make_unique<filter::expressions::StringSearch>(
         std::move(receiver_name), std::move(regex)
      );
   }

   if (call.method_name == "isNull") {
      return std::make_unique<filter::expressions::IsNull>(std::move(receiver_name));
   }

   throw ParseException(
      call.receiver->location,
      "Unknown filter method: {}",
      call.method_name
   );
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

      if (call.function_name == "hasMutation") {
         return std::make_unique<filter::expressions::HasMutation<Nucleotide>>(
            sequence_name, static_cast<uint32_t>(position.value())
         );
      }
      return std::make_unique<filter::expressions::HasMutation<AminoAcid>>(
         sequence_name, static_cast<uint32_t>(position.value())
      );
   }

   if (call.function_name == "nOf") {
      if (call.arguments.size() < 2) {
         throw ParseException(SourceLocation{}, "nOf() requires at least 2 arguments");
      }

      auto* n_lit = std::get_if<ast::IntLiteral>(&call.arguments[0].value->value);
      if (!n_lit) {
         throw ParseException(
            call.arguments[0].location,
            "First argument to nOf() must be an integer"
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

   throw ParseException(
      SourceLocation{},
      "Unknown function: {}",
      call.function_name
   );
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
            throw ParseException(
               ast_expr.location,
               "Unsupported binary operator in filter: {}",
               ast::binaryOpToString(node.op)
            );
         } else if constexpr (std::is_same_v<T, ast::UnaryNotExpr>) {
            return std::make_unique<filter::expressions::Negation>(
               convertToFilter(*node.operand)
            );
         } else if constexpr (std::is_same_v<T, ast::MethodCall>) {
            return convertMethodCallToFilter(node);
         } else if constexpr (std::is_same_v<T, ast::FunctionCall>) {
            return convertFunctionCallToFilter(node);
         } else if constexpr (std::is_same_v<T, ast::BoolLiteral>) {
            if (node.value) {
               return std::make_unique<filter::expressions::True>();
            }
            return std::make_unique<filter::expressions::Negation>(
               std::make_unique<filter::expressions::True>()
            );
         } else if constexpr (std::is_same_v<T, ast::Identifier>) {
            // A bare identifier used as a filter expression — treat as boolean column check
            return std::make_unique<filter::expressions::BoolEquals>(node.name, true);
         } else {
            throw ParseException(
               ast_expr.location,
               "Cannot convert expression to filter: {}",
               ast_expr.toString()
            );
         }
      },
      ast_expr.value
   );
}

std::unique_ptr<actions::Action> convertToAction(const ast::MethodCall& method_call) {
   if (method_call.method_name == "aggregated") {
      std::vector<std::string> group_by;
      for (const auto& arg : method_call.arguments) {
         if (auto* id = std::get_if<ast::Identifier>(&arg.value->value)) {
            group_by.push_back(id->name);
         } else if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            group_by.push_back(str->value);
         }
      }
      return std::make_unique<actions::Aggregated>(std::move(group_by));
   }

   if (method_call.method_name == "details") {
      std::vector<std::string> fields;
      auto limit = findNamedIntArg(method_call.arguments, "limit");
      auto offset = findNamedIntArg(method_call.arguments, "offset");

      for (const auto& arg : method_call.arguments) {
         if (arg.name.has_value() && (arg.name.value() == "limit" || arg.name.value() == "offset")
         ) {
            continue;
         }
         if (auto* id = std::get_if<ast::Identifier>(&arg.value->value)) {
            fields.push_back(id->name);
         } else if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            fields.push_back(str->value);
         }
      }
      auto action = std::make_unique<actions::Details>(std::move(fields));
      if (limit.has_value() || offset.has_value()) {
         action->setOrdering(
            {},
            limit.has_value() ? std::optional<uint32_t>(static_cast<uint32_t>(limit.value()))
                              : std::nullopt,
            offset.has_value() ? std::optional<uint32_t>(static_cast<uint32_t>(offset.value()))
                               : std::nullopt,
            std::nullopt
         );
      }
      return action;
   }

   if (method_call.method_name == "mutations") {
      std::vector<std::string> sequence_names;
      double min_proportion = 0.05;

      auto min_prop = findNamedStringArg(method_call.arguments, "minProportion");
      if (min_prop.has_value()) {
         min_proportion = std::stod(min_prop.value());
      }

      for (const auto& arg : method_call.arguments) {
         if (arg.name.has_value() && arg.name.value() == "minProportion") {
            continue;
         }
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      return std::make_unique<actions::Mutations<Nucleotide>>(
         std::move(sequence_names), min_proportion, std::vector<std::string_view>{}
      );
   }

   if (method_call.method_name == "fasta") {
      std::vector<std::string> sequence_names;
      std::vector<std::string> additional_fields;
      for (const auto& arg : method_call.arguments) {
         if (auto* str = std::get_if<ast::StringLiteral>(&arg.value->value)) {
            sequence_names.push_back(str->value);
         }
      }
      return std::make_unique<actions::Fasta>(
         std::move(sequence_names), std::move(additional_fields)
      );
   }

   throw IllegalQueryException("Unknown action: {}", method_call.method_name);
}

std::shared_ptr<query_engine::Query> convertToQuery(const ast::Expression& ast_expr) {
   // Expected shape: table.filter(...).action(...)
   // or: table.action(...)
   if (auto* outer_call = std::get_if<ast::MethodCall>(&ast_expr.value)) {
      // Check if this is an action method
      bool is_action = outer_call->method_name == "aggregated" ||
                       outer_call->method_name == "details" ||
                       outer_call->method_name == "mutations" ||
                       outer_call->method_name == "fasta";

      if (is_action) {
         auto action = convertToAction(*outer_call);

         // Check if receiver is filter call
         if (auto* filter_call = std::get_if<ast::MethodCall>(&outer_call->receiver->value)) {
            if (filter_call->method_name == "filter") {
               if (filter_call->arguments.size() != 1) {
                  throw ParseException(
                     filter_call->receiver->location,
                     "filter() requires exactly 1 argument"
                  );
               }
               auto filter = convertToFilter(*filter_call->arguments[0].value);
               return std::make_shared<query_engine::Query>(
                  std::move(filter), std::move(action)
               );
            }
         }

         // No filter — use True
         auto filter = std::make_unique<filter::expressions::True>();
         return std::make_shared<query_engine::Query>(std::move(filter), std::move(action));
      }
   }

   throw ParseException(
      ast_expr.location,
      "Query must end with an action (e.g., .aggregated(), .details())"
   );
}

}  // namespace silo::query_engine::saneql
