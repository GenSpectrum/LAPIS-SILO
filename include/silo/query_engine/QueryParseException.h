#ifndef SILO_INCLUDE_SILO_QUERY_ENGINE_QUERYPARSEEXCEPTION_H_
#define SILO_INCLUDE_SILO_QUERY_ENGINE_QUERYPARSEEXCEPTION_H_

#include <iostream>

namespace silo {

class [[maybe_unused]] QueryParseException : public std::runtime_error {
  public:
   [[maybe_unused]] QueryParseException(const std::string& error_message);
};
}  // namespace silo

#endif  // SILO_INCLUDE_SILO_QUERY_ENGINE_QUERYPARSEEXCEPTION_H_
