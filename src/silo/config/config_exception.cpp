#include "silo/config/config_exception.h"

namespace silo {

ConfigException::ConfigException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo
