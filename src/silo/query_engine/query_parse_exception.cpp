#include "silo/query_engine/query_parse_exception.h"

#include <stdexcept>
#include <string>

namespace silo {
[[maybe_unused]] QueryParseException::QueryParseException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}
}  // namespace silo