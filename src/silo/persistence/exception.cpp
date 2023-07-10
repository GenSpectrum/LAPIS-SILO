#include "silo/persistence/exception.h"

#include <string>

namespace silo::persistence {

LoadDatabaseException::LoadDatabaseException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

SaveDatabaseException::SaveDatabaseException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo::persistence