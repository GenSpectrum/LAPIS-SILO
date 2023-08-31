#pragma once

#include <stdexcept>
#include <string>

namespace silo {

class [[maybe_unused]] QueryCompilationException : public std::runtime_error {
  public:
   [[maybe_unused]] QueryCompilationException(const std::string& error_message);
};
}  // namespace silo
