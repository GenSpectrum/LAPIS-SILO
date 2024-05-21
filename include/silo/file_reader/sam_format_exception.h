#pragma once

#include <stdexcept>
#include <string>

namespace silo {

class SamFormatException : public std::runtime_error {
  public:
   explicit SamFormatException(const std::string& error_message);
};

}  // namespace silo
