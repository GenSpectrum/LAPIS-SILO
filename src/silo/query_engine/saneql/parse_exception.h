#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include "silo/query_engine/saneql/source_location.h"

namespace silo::query_engine::saneql {

class ParseException : public std::runtime_error {
   SourceLocation location;

  public:
   explicit ParseException(const std::string& message, SourceLocation location = {});

   template <typename... Args>
   explicit ParseException(
      SourceLocation location,
      fmt::format_string<Args...> fmt_str,
      Args&&... args
   )
       : std::runtime_error(fmt::format(
            "Parse error at {}: {}",
            location.toString(),
            fmt::format(fmt_str, std::forward<Args>(args)...)
         )),
         location(location) {}

   [[nodiscard]] SourceLocation getLocation() const { return location; }
};

}  // namespace silo::query_engine::saneql
