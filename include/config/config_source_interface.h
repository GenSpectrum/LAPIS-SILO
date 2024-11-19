#pragma once

//! Interfaces for configuration access.
//!
//! The goals are:
//!
//!   * allow for configuration files (e.g. YAML), environment
//!     variables, and command line options, and have them shadow
//!     (override) each other (in this order).
//!   * report any I/O errors (e.g. when reading YAML files or
//!     decoding unicode), obviously.
//!   * report unknown configuration keys.
//!   * report invalid configuration value formats.
//!
//! To achieve these goals, each of those configuration sources (YAML
//! or perhaps other kinds of files, env vars, command line arguments)
//! needs readers that implement the interfaces here. Each reader goes
//! through these steps:
//!
//!  0. Optionally have a parameterless type that only implements
//!     option key path formatting.
//!
//!  1. Have a first stage object that contains the result of reading
//!     the actual source (e.g. file) and reporting I/O errors;
//!     there's no interface for this since this is specific to each
//!     source.
//!
//!  2. The VerifyConfigSource::verify function that this object
//!     implements checks that all found keys are OK, and returns a
//!     VerifiedConfigSource object.
//!
//!  3. To fill in a to be configured struct, the
//!     `VerifiedConfigSource` object is queried for each field key
//!     and the struct field is set via OverwriteFrom::overwrite_from
//!     implemented on the struct in question. `VerifiedConfigSource`
//!     is also implemented for `ConfigStruct`, that way
//!     overwrite_from can also be used identically to initialize a
//!     struct with the default values. (Note: the configuration
//!     structs need to implement `Default`, too, so that they can be
//!     created first for the following chain of side effects; but
//!     this just sets them to the defaults for each contained data
//!     type via derive. Be careful not to forget to overwrite from
//!     the ConfigStruct!)
//!
//! To reiterate, step 3 is applied to a particular struct first for
//! the defaults and then for all config sources in order to achieve
//! the shadowing effect (via `overwrite_from`).
//!
//! The information about valid keys as well as optional default
//! values for them is declared via
//! `super::config_metadata::ConfigStruct` values. They contain only a
//! single-string key for each field (representing one config key path
//! segment), and the full path for each field is constructed from
//! the nesting of the ConfigStruct instances (a tree).
//!

#include <span>
#include <string>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>
#include <boost/lexical_cast/bad_lexical_cast.hpp>

#include <config/config_key_path.h>
#include <silo/common/cons_list.h>
#include <silo/common/panic.h>
#include <silo/config/util/config_exception.h>
#include "config/config_value.h"
#include "silo/common/type_name.h"

/* #[error("error in {config_context}: unknown key(s) {invalid_config_keys:?}")] */
struct InvalidConfigKeyError {
   std::string config_context;
   std::vector<std::string> invalid_config_keys;
};
/* #[error("error in {config_context}: {message}")] */
struct ParseError {
   std::string config_context;
   std::string message;
};

/// Config keys (represented via the type `ConfigKeyPath`) are lists
/// of strings in camel case, and used as such in yaml config
/// files. For command line arguments those are translated to kebab
/// case (lower-case joined '-' before uppercase characters), for
/// environment variables to uppercase with underscores and prefixed
/// with "SILO_". Multi-segment paths are treated as nested
/// dictionaries in yaml config files, joined with '-' for command
/// line arguments and '_' for environment variables. `ConfigSource`
/// provides the means to do this type-specific conversion
class ConfigSource {
  public:
   /// A human-readable description including type (command line,
   /// config file, env var) and if applicable path to the file.
   // used to be called `configType`
   [[nodiscard]] virtual std::string configContext() const = 0;

   /// Convert a config key path to a string for this kind of config
   /// source.
   [[nodiscard]] virtual std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const = 0;

   virtual ~ConfigSource() = default;
};

class VerifiedConfigSource;

/// A ConfigSource is providing I/O-error free access to a set of
/// unverified configuration data.
class VerifyConfigSource : public ConfigSource /* XX does it make sense to request that here? */ {
  public:
   /// Verify that all user-presented *keys* in `self` are
   /// valid. (Correctness check of the *values* only happens later
   /// via `get`.) Throws [silo::config::ConfigException] on
   /// verification errors (you could subclass those as
   /// InvalidConfigKeyError, ParseError). May consume/move `this`.
   [[nodiscard]] virtual std::unique_ptr<VerifiedConfigSource> verify(
      const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
   ) = 0;

   /// Utility method: turn a vector of (ConfigKeyPath, ConfigValue)
   /// into one of (std::string, ConfigValue) where the string is from
   /// `configKeyPathToString`.
   [[nodiscard]] std::unordered_map<std::string, const ConfigValue*> stringifiedKeyToConfigMap(
      const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
   ) const;

   virtual ~VerifyConfigSource() = default;
};

/// A VerifiedConfigSource is providing I/O- and key error free (but
/// not necessarily value-error free) access to a set of configuration
/// data.
class VerifiedConfigSource : public ConfigSource {
  public:
   /// Retrieve a config value for the given key as a string
   /// (potentially converting other value types). (Explicitly
   /// getting as a string is necessary for YAML, where the YAML
   /// parser already has some typed representations but not
   /// necessarily those we need. (Todo: this is a hack, improve.))
   /// This returns an option since even though invalid options are
   /// not present in self, the given option may also not be present.
   [[nodiscard]] virtual std::optional<std::string> getString(const ConfigKeyPath& config_key_path
   ) const = 0;

   /// Get positional arguments, if any (only used for command-line
   /// arguments)
   [[nodiscard]] virtual const std::vector<std::string>* positionalArgs() const = 0;

   // For compatibility only, XXX still needed?
   [[nodiscard]] bool hasProperty(const ConfigKeyPath& config_key_path) const {
      return getString(config_key_path).has_value();
   }

   virtual ~VerifiedConfigSource() = default;
};

namespace config::config_source_interface {

// `get` and `set` are standalone functions because virtual (in Rust,
// object trait) methods can't take type parameters.

/// Try to get the parsed representation of a value, if present. Use
/// the `set` wrapper to set struct fields for convenience. Throws
/// [silo::config::ConfigException] on parsing errors.
template <typename T /* : FromStr */>
std::optional<T> get(
   const VerifiedConfigSource& source,
   const ConsList<std::string>& parents,
   const char* key
) {
   ConfigKeyPath config_key_path{parents.cons(key).toVecReverse()};
   auto value_string = source.getString(config_key_path);
   if (!value_string) {
      return std::nullopt;
   }
   try {
      return std::optional{boost::lexical_cast<T>(*value_string)};
   } catch (boost::bad_lexical_cast& e) {
      const std::string error_message = fmt::format(
         "could not parse the value '{}' from {} option '{}' to type '{}'",
         *value_string,
         source.configContext(),
         source.configKeyPathToString(config_key_path),
         silo::common::typeName<T>()
      );
      // SPDLOG_ERROR(error_message);
      throw silo::config::ConfigException(error_message);
   }
}

/// Set `out` to the value for the given option, if present and
/// parseable. Returns whether `out` was overwritten. Throws
/// [silo::config::ConfigException] on parsing errors.
template <typename T /*: FromStr + Debug */, typename Outer>
bool set(
   Outer& out,
   const VerifiedConfigSource& source,
   const ConsList<std::string>& parents,
   const char* key
) {
   std::optional<T> value = get<T>(source, parents, key);
   if (!value.has_value()) {
      return false;
   }
   SPDLOG_TRACE(
      "setting config key {} to '{}' from {}",
      ConfigKeyPath(parents.cons(key).toVecReverse()).toDebugString(),
      *value,
      source.configContext()
   );
   out = std::move(value.value());
   return true;
}

}  // namespace config::config_source_interface
