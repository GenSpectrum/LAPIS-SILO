#ifndef SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_COMPILATION_EXCEPTION_H_
#define SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_COMPILATION_EXCEPTION_H_

#include <iostream>
#include <string>

namespace silo {

class [[maybe_unused]] QueryCompilationException : public std::runtime_error {
  public:
   [[maybe_unused]] QueryCompilationException(const std::string& error_message);
};
}  // namespace silo

#endif  // SILO_INCLUDE_SILO_QUERY_ENGINE_QUERY_COMPILATION_EXCEPTION_H_
