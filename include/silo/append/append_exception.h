#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace silo::append {

class AppendException : public std::runtime_error {
  public:
   explicit AppendException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit AppendException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::append
