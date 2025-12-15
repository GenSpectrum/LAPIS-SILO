#pragma once

#include <exception>
#include <typeinfo>

#include <fmt/format.h>

namespace silo::api {

class [[maybe_unused]] BadRequest : public std::runtime_error {
  public:
   explicit BadRequest(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit BadRequest(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace silo::api
