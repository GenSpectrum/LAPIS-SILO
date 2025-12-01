#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace silo::initialize {

class InitializeException : public std::runtime_error {
  public:
   explicit InitializeException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit InitializeException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::initialize
