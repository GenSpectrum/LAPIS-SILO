#include "config/config_exception.h"

#include <string>

namespace rhydb::config {

ConfigException::ConfigException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace rhydb::config
