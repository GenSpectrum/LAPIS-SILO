#pragma once

#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <boost/functional/hash.hpp>

namespace silo::config {

/// Internal representation of config keys.
/// List of lists of _non-empty lower-case alphanumeric_ strings
/// By example the YAML field:
/// `query.materializationCutoff` will be represented as
/// ["query",["materialization","cutoff"]]
/// This is easy to handle internally and also easy for transformation
/// into CLI argument string and environment variable string
///
/// Note: to print `ConfigKeyPath`s, decide on a representation,
/// probably the Yaml one, and call its `configKeyPathToString`.
class ConfigKeyPath {
   explicit ConfigKeyPath(std::vector<std::vector<std::string>> path)
       : path(std::move(path)) {}

   std::vector<std::vector<std::string>> path;

  public:
   ConfigKeyPath() = default;

   [[nodiscard]] std::vector<std::vector<std::string>> getPath() const;

   static std::optional<ConfigKeyPath> tryFrom(const std::vector<std::vector<std::string>>& paths);

   friend bool operator==(const ConfigKeyPath& lhs, const ConfigKeyPath& rhs) {
      return lhs.path == rhs.path;
   }
};

/// Like ConfigKeyPath, but it is impossible to know if a split point
/// refers to a path segment or sub-path segment (i.e. to decide
/// whether the input value meant to refer to api.port or apiPort), so
/// we have only 1 level of strings. This is the case for CLI
/// arguments (--api-port) and Environment Variables (SILO_API_PORT)
class AmbiguousConfigKeyPath {
   std::vector<std::string> path;

  public:
   static std::optional<AmbiguousConfigKeyPath> tryFrom(std::vector<std::string>&& path);

   static AmbiguousConfigKeyPath from(const ConfigKeyPath& key_path);

   friend bool operator==(const AmbiguousConfigKeyPath& lhs, const AmbiguousConfigKeyPath& rhs) {
      return lhs.path == rhs.path;
   }
};

}  // namespace silo::config

// So that we are able to use std::unordered_map of our internal representation of config keys
namespace std {
template <>
struct hash<silo::config::ConfigKeyPath> {
   std::size_t operator()(const silo::config::ConfigKeyPath& key) const;
};
}  // namespace std
