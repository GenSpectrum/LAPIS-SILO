#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace silo::preprocessing {

class PreprocessingException : public std::runtime_error {
  public:
   explicit PreprocessingException(const std::string& error_message);

   template <typename... Args>
   explicit PreprocessingException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::preprocessing
