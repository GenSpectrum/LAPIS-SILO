#ifndef SILO_NUMERIC_CONVERSION_H
#define SILO_NUMERIC_CONVERSION_H

#include <stdexcept>
#include <string>

namespace silo {

class NumericConversionException : public std::runtime_error {
  public:
   explicit NumericConversionException(const std::string& error_message);
};

uint32_t tryConvertStringToU32(const std::string& input);

}  // namespace silo

#endif  // SILO_NUMERIC_CONVERSION_H
