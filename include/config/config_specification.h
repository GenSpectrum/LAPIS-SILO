#pragma once

//! Structs with which to declare metainformation on structs that are
//! to hold configuration data.

#include <optional>
#include <span>
#include <variant>

#include "config/config_key_path.h"
#include "config/config_value.h"
#include "config/verified_config_source.h"
#include "silo/common/cons_list.h"

namespace silo::config {

/// Does not support extracting non-option arguments; those wouldn't
/// be supported by env vars or config files anyway, although could
/// still be specified for command line, but that's not implemented
/// currently.
class ConfigSpecification {
  public:
   /// The name of the program for which this config is used. This will be printed in the help text
   std::string_view program_name;
   // Using std::vector so that initialization in place is possible;
   // std::span would require the array to exist in a different global
   // first, don't want to make it verbose like that. Paying with
   // dropping constexpr for that.
   std::vector<ConfigValueSpecification> fields;

   std::string helpText() const;

   std::optional<ConfigValueSpecification> getValueSpecification(const ConfigKeyPath& key) const;

   std::optional<ConfigValueSpecification> getValueSpecificationFromAmbiguousKey(
      const AmbiguousConfigKeyPath& key
   ) const;

   VerifiedConfigSource getConfigSourceFromDefaults() const;
};

}  // namespace silo::config
