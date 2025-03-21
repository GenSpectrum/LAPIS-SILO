#pragma once

#include <stdexcept>
#include <string>

namespace silo::initialize {

class InitializeException : public std::runtime_error {
  public:
   explicit InitializeException(const std::string& error_message)
       : std::runtime_error(error_message) {}
};

}  // namespace silo::initialize
