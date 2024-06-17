#include "silo/file_reader/sam_format_exception.h"

#include <stdexcept>
#include <string>

namespace silo {

SamFormatException::SamFormatException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo