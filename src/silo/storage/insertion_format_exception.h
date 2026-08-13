#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace rhydb::storage {

class InsertionFormatException : public std::runtime_error {
  public:
   explicit InsertionFormatException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit InsertionFormatException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace rhydb::storage
