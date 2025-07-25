#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#define CHECK_SILO_QUERY(condition, ...)   \
   if (!(condition)) {                     \
      throw silo::BadRequest(__VA_ARGS__); \
   }

namespace silo {

class [[maybe_unused]] BadRequest : public std::runtime_error {
  public:
   explicit BadRequest(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit BadRequest(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo
