#pragma once

#define CONCAT(lhs, rhs) lhs##rhs

#define RETURN_NOT_OK(expected)     \
   do {                             \
      if (!(expected).has_value())  \
         return (expected).error(); \
   } while (false)

#define ASSIGN_OR_RAISE_NAME(x, y) CONCAT(x, y)

// NOLINTBEGIN(bugprone-macro-parentheses)
#define ASSIGN_OR_RAISE_IMPL(expected_name, lhs, rexpr) \
   auto&& expected_name = (rexpr);                      \
   if (!(expected_name).has_value())                    \
      return std::unexpected{(expected_name).error()};  \
   lhs = std::move(expected_name).value();
// NOLINTEND(bugprone-macro-parentheses)

#define ASSIGN_OR_RAISE(lhs, rexpr) \
   ASSIGN_OR_RAISE_IMPL(ASSIGN_OR_RAISE_NAME(_error_or_value, __COUNTER__), lhs, rexpr);

#define ASSIGN_OR_RAISE_SIMDJSON_IMPL(error_var, lhs_type, lhs_var, rexpr, ...) \
   lhs_type lhs_var;                                                            \
   auto error_var = (rexpr).get(lhs_var);                                       \
   if (error_var)                                                               \
      return std::unexpected{fmt::format(__VA_ARGS__, simdjson::error_message(error_var))};

#define ASSIGN_OR_RAISE_SIMDJSON(lhs_type, lhs_var, rexpr, ...)                                 \
   ASSIGN_OR_RAISE_SIMDJSON_IMPL(                                                               \
      ASSIGN_OR_RAISE_NAME(simdjson_error_, __COUNTER__), lhs_type, lhs_var, rexpr, __VA_ARGS__ \
   );
