#pragma once

#include <stdexcept>
#include <string>

namespace silo::common {

class DateFormatException : public std::runtime_error {
  public:
   explicit DateFormatException(const std::string& error_message);
};

}  // namespace silo::common
