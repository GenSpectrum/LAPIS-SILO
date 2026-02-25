#pragma once

#include <cstdint>
#include <string>

#include <fmt/format.h>

namespace silo::query_engine::saneql {

struct SourceLocation {
   uint32_t line = 1;
   uint32_t column = 1;

   [[nodiscard]] std::string toString() const { return fmt::format("{}:{}", line, column); }
};

struct SourceRange {
   SourceLocation start;
   SourceLocation end;
};

}  // namespace silo::query_engine::saneql
