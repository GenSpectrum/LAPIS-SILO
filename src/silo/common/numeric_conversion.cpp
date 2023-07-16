#include "silo/common/numeric_conversion.h"

#include <sstream>

namespace silo {

NumericConversionException::NumericConversionException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

uint32_t tryConvertStringToU32(const std::string& input) {
   std::stringstream string_number_input(input);
   uint32_t target;
   if (!(string_number_input >> target)) {
      const std::string message = "Failed to convert \"" + input + "\" to uint32";
      throw NumericConversionException(message);
   }
   return target;
}

}  // namespace silo