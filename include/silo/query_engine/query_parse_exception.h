#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace silo {

class [[maybe_unused]] QueryParseException : public std::runtime_error {
  public:
   [[maybe_unused]] QueryParseException(const std::string& error_message);
};
}  // namespace silo
