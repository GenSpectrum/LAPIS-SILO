#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#define CHECK_SILO_QUERY(condition, ...)                               \
   do {                                                                \
      const bool condition_bool = condition;                           \
      if (!(condition_bool)) {                                         \
         throw silo::query_engine::IllegalQueryException(__VA_ARGS__); \
      }                                                                \
   } while (0);

namespace silo::query_engine {

class [[maybe_unused]] IllegalQueryException : public std::runtime_error {
  public:
   explicit IllegalQueryException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit IllegalQueryException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::query_engine
