#include "silo/query_engine/bad_request.h"

#include <stdexcept>
#include <string>

namespace silo {

BadRequest::BadRequest(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

}  // namespace silo
