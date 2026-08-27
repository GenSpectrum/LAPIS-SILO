#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace rhydb::append::bam {

/// Raised when a BAM/BGZF input is malformed or cannot be decoded. Surfaces
/// through the normal append path, where it is turned into an AppendException.
class BamException : public std::runtime_error {
  public:
   explicit BamException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit BamException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace rhydb::append::bam
