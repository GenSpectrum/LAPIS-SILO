#include "silo/persistence/load_database_exception.h"

#include <iostream>
#include <string>

namespace silo {
namespace persistence {

LoadDatabaseException::LoadDatabaseException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace persistence
}  // namespace silo