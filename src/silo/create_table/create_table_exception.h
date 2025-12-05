#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace silo::create_table {

class CreateTableException : public std::runtime_error {
  public:
   explicit CreateTableException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit CreateTableException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::create_table
