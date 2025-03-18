#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace silo::schema {

class DuplicatePrimaryKeyException : public std::runtime_error {
  public:
   explicit DuplicatePrimaryKeyException(const std::string& error_message);

   template <typename... Args>
   explicit DuplicatePrimaryKeyException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::schema
