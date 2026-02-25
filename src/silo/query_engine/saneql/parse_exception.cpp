#include "silo/query_engine/saneql/parse_exception.h"

#include <fmt/format.h>

namespace silo::query_engine::saneql {

ParseException::ParseException(const std::string& message, SourceLocation location)
    : std::runtime_error(fmt::format("Parse error at {}: {}", location.toString(), message)),
      location(location) {}

}  // namespace silo::query_engine::saneql
