#pragma once

#include <string>
#include <vector>

/// List of hierarchical option path segments, each of which in
/// camel case.
// This was called AbstractConfigSource::Option in C++
/* #[derive(Debug, Clone, PartialEq, Eq, Hash)] */
class ConfigKeyPath {
   // The Rust version uses `Vec<Cow<'static, str>>`, but we can rely
   // on toplevel runtime initialization in C++ (no need to be
   // constexpr).
  public:
   std::vector<std::string> path;

   friend bool operator==(const ConfigKeyPath& lhs, const ConfigKeyPath& rhs) {
      return lhs.path == rhs.path;
   }

   [[nodiscard]] std::string toDebugString() const;
};
