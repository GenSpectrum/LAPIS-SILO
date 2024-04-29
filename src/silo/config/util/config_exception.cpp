#include "silo/config/util/config_exception.h"

#include <string>

namespace silo::config {

ConfigException::ConfigException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo::config
