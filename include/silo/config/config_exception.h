#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

namespace silo::config {

class ConfigException : public std::runtime_error {
  public:
   explicit ConfigException(const std::string& error_message);
};

}  // namespace silo::config
