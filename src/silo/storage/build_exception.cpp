#include "silo/storage/build_exception.h"

#include <string>

namespace silo {

BuildException::BuildException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo
