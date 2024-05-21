#pragma once

#include <stdexcept>
#include <string>

namespace silo {

class FastaFormatException : public std::runtime_error {
  public:
   explicit FastaFormatException(const std::string& error_message);
};

}  // namespace silo
