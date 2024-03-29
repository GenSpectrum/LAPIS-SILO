#include "silo/common/date_format_exception.h"

#include <stdexcept>
#include <string>

namespace silo::common {

DateFormatException::DateFormatException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo::common
