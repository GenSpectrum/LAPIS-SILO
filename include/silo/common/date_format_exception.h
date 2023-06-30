#ifndef SILO_DATE_FORMAT_EXCEPTION_H
#define SILO_DATE_FORMAT_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace silo::common {

class DateFormatException : public std::runtime_error {
  public:
   explicit DateFormatException(const std::string& error_message);
};

}  // namespace silo::common

#endif  // SILO_DATE_FORMAT_EXCEPTION_H
