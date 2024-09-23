#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace silo::config {

/// A source of config values (from a config file, env vars, or
/// command line arguments).
///
/// Values may be loaded at instantiation time of the object
/// implementing `AbstractConfigSource` (as is the case for
/// `YamlFile`), or be retrieved from the environment at query time
/// (the use of the get* methods) (as is the case for
/// `EnvironmentVariables`) or held via reference from
/// instantiation time (as is the case for `CommandLineArguments`).
///
/// Config keys (represented via class `Option`) are lists of strings
/// in camel case, and used as such in yaml config files. For command
/// line arguments those are translated to kebab case (lower-case
/// joined '-' before uppercase characters), for environment variables
/// to uppercase with underscores and prefixed with
/// "SILO_". Multi-segment paths are treated as nested dictionaries in
/// yaml config files, joined with '-' for command line arguments and
/// '_' for environment variables.
class AbstractConfigSource {
  public:
   class Option {
     public:
      /// List of hierarchical option path segments, each of which in
      /// camel case.
      std::vector<std::string> access_path;

      /// `access_path` joined with ".".
      [[nodiscard]] std::string toString() const;
   };

   /// A human-readable description including type (command line,
   /// config file, env var) and if applicable path to the file.
   [[nodiscard]] virtual std::string configType() const = 0;

   /// Check if a value is available for the given key.
   [[nodiscard]] virtual bool hasProperty(const Option& option) const = 0;
   /// Retrieve a config value for the given key as a string
   /// (potentially converting other value types).
   [[nodiscard]] virtual std::optional<std::string> getString(const Option& option) const = 0;
   [[nodiscard]] virtual std::optional<int32_t> getInt32(const Option& option) const;
   [[nodiscard]] virtual std::optional<uint16_t> getUInt16(const Option& option) const;
   [[nodiscard]] virtual std::optional<uint32_t> getUInt32(const Option& option) const;
   [[nodiscard]] virtual std::optional<uint64_t> getUInt64(const Option& option) const;
};

}  // namespace silo::config
