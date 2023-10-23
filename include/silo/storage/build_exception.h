#pragma once

#include <stdexcept>
#include <string>

namespace silo {

class BuildException : public std::runtime_error {
  public:
   explicit BuildException(const std::string& error_message);
};

}  // namespace silo
