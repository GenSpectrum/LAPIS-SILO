#include "rhydb/query_engine/query_compilation_exception.h"

#include <stdexcept>
#include <string>

namespace rhydb::query_engine {
[[maybe_unused]] QueryCompilationException::QueryCompilationException(
   const std::string& error_message
)
    : std::runtime_error(error_message.c_str()) {}
}  // namespace rhydb::query_engine