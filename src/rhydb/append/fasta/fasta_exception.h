#pragma once

#include <stdexcept>
#include <string>

#include <fmt/core.h>

namespace rhydb::append::fasta {

/// Raised when a FASTA input is malformed. Surfaces through the normal append
/// path, where it is turned into an AppendException.
class FastaException : public std::runtime_error {
  public:
   explicit FastaException(const std::string& error_message)
       : std::runtime_error(error_message) {}

   template <typename... Args>
   explicit FastaException(fmt::format_string<Args...> fmt_str, Args&&... args)
       : std::runtime_error(fmt::format(fmt_str, std::forward<Args>(args)...)) {}
};

}  // namespace rhydb::append::fasta
