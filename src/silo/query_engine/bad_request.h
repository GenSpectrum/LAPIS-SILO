#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#define CHECK_SILO_QUERY(condition, ...)                \
   if (!(condition)) {                                  \
      throw silo::BadRequest(fmt::format(__VA_ARGS__)); \
   }

namespace silo {

class [[maybe_unused]] BadRequest : public std::runtime_error {
  public:
   explicit BadRequest(const std::string& error_message);
};

}  // namespace silo
