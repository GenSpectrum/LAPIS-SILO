#pragma once

#include "config/config_specification.h"
#include "config/verified_config_source.h"

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
//! To achieve these goals, each of those configuration backends (YAML
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
//!  2. The ConfigBackend::verify function that this object implements
//!     checks that all found keys are OK and specified for the
//!     desired config type, and returns a VerifiedConfigSource
//!     object.
//!
//!  3. To fill in a to be configured struct, the
//!     `VerifiedConfigSource` object is queried for each field key
//!     and the struct field is set via Config::overwriteFrom
//!     implemented on the struct in question. `VerifiedConfigSource`
//!     is also implemented for `ConfigSpecification`, that way
//!     overwriteFrom can also be used identically to initialize a
//!     struct with the default values.
//!
//! To reiterate, step 3 is applied to a particular struct first for
//! the defaults and then for all config sources in order to achieve
//! the shadowing effect (via `overwriteFrom`).
//!
//! The information about valid keys as well as optional default
//! values for them is declared via
//! `config::ConfigSpecification` values. They contain the
//! ConfigKeyPath, type, default value and help text for each field

namespace silo::config {

/// Config keys (represented via the type `ConfigKeyPath`) have an internal representation.
/// They are printed (and parsed lists) in camel case in the yaml config
/// files. For command line arguments those are translated to kebab
/// case (lower-case joined '-' before uppercase characters), for
/// environment variables to uppercase with underscores and prefixed
/// with "SILO_". Multi-segment paths are treated as nested
/// dictionaries in yaml config files, joined with '-' for command
/// line arguments and '_' for environment variables. Each `ConfigBackend`
/// provides the means to do these type-specific conversions

/// A ConfigBackend is providing I/O-error free access to a set of
/// unverified configuration data.
class ConfigBackend {
  public:
   /// Verify that all user-presented *keys* in `self` are
   /// valid and the types of corresponding *values* resolved.
   [[nodiscard]] virtual VerifiedConfigSource verify(const ConfigSpecification& config_specification
   ) const = 0;
};

}  // namespace silo::config
