#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace silo::query_engine {

class [[maybe_unused]] QueryCompilationException : public std::runtime_error {
  public:
   [[maybe_unused]] explicit QueryCompilationException(const std::string& error_message);

   template <typename... Args>
   explicit QueryCompilationException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};
}  // namespace silo::query_engine
