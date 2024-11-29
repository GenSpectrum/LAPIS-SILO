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

class ConfigValueSpecification {
   ConfigValueSpecification() = default;

  public:
   ConfigKeyPath key;
   ConfigValueType type;
   std::optional<ConfigValue> default_value;
   /// Help as shown for --help, excluding the other info above.
   /// If type is bool, the command line option does not take an argument but
   /// is the constant "true", which will be added to the help text
   std::string_view help_text;

   [[nodiscard]] ConfigValue getValueFromString(std::string value_string) const;

   static ConfigValueSpecification createWithoutDefault(
      ConfigKeyPath key,
      ConfigValueType value_type,
      std::string_view help_text
   ) {
      ConfigValueSpecification value_specification;
      value_specification.key = std::move(key);
      value_specification.type = value_type;
      value_specification.help_text = help_text;
      return value_specification;
   }

   /// No need for the value_type. It is implicitly defined by the default. Prevents
   /// misspecification.
   static ConfigValueSpecification createWithDefault(
      ConfigKeyPath key,
      const ConfigValue& default_value,
      std::string_view help_text
   ) {
      ConfigValueSpecification value_specification;
      value_specification.key = std::move(key);
      value_specification.type = default_value.getValueType();
      value_specification.default_value = default_value;
      value_specification.help_text = help_text;
      return value_specification;
   }
};

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

   [[nodiscard]] std::string helpText() const;

   [[nodiscard]] std::optional<ConfigValueSpecification> getValueSpecification(
      const ConfigKeyPath& key
   ) const;

   [[nodiscard]] std::optional<ConfigValueSpecification> getValueSpecificationFromAmbiguousKey(
      const AmbiguousConfigKeyPath& key
   ) const;

   /// Convert the ConfigSpecification to a VerifiedConfigSource, to
   /// use as the source for the default values.
   [[nodiscard]] VerifiedConfigSource getConfigSourceFromDefaults() const;
};

}  // namespace silo::config
