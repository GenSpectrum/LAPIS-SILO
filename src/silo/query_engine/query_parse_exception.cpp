#include "silo/query_engine/query_parse_exception.h"

#include <stdexcept>
#include <string>

namespace silo {

QueryException::QueryException(const std::string& error_message)
    : std::runtime_error(error_message.c_str()) {}

[[maybe_unused]] QueryParseException::QueryParseException(const std::string& error_message)
    : QueryException(error_message) {}

std::string_view QueryParseException::duringString() const {
   return "parsing";
}

[[maybe_unused]] QueryEvaluationException::QueryEvaluationException(const std::string& error_message
)
    : QueryException(error_message) {}

std::string_view QueryEvaluationException::duringString() const {
   return "evaluation";
}

}  // namespace silo
