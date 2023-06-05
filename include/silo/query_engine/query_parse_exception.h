#ifndef SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_PARSE_EXCEPTION_H_
#define SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_PARSE_EXCEPTION_H_

#include <iostream>
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

#endif  // SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_PARSE_EXCEPTION_H_
