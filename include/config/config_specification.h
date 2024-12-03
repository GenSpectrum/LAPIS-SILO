#pragma once

//! Structs with which to declare metainformation on structs that are
//! to hold configuration data.

#include <optional>
#include <span>
#include <variant>

#include "config/config_key_path.h"
#include "config/config_value.h"
#include "config/verified_config_attributes.h"
#include "silo/common/cons_list.h"

namespace silo::config {

class ConfigAttributeSpecification {
   ConfigAttributeSpecification() = default;

  public:
   ConfigKeyPath key;
   ConfigValueType type;
   std::optional<ConfigValue> default_value;
   /// Help as shown for --help, excluding the other info above.
   /// If type is bool, the command line option does not take an argument but
   /// is the constant "true", which will be added to the help text
   std::string_view help_text;

   [[nodiscard]] ConfigValue parseValueFromString(std::string value_string) const;

   static ConfigAttributeSpecification createWithoutDefault(
      ConfigKeyPath key,
      ConfigValueType value_type,
      std::string_view help_text
   ) {
      ConfigAttributeSpecification attribute_spec;
      attribute_spec.key = std::move(key);
      attribute_spec.type = value_type;
      attribute_spec.help_text = help_text;
      return attribute_spec;
   }

   static ConfigAttributeSpecification createWithDefault(
      ConfigKeyPath key,
      const ConfigValue& default_value,
      std::string_view help_text
   ) {
      ConfigAttributeSpecification attribute_spec;
      attribute_spec.key = std::move(key);
      attribute_spec.type = default_value.getValueType();
      attribute_spec.default_value = default_value;
      attribute_spec.help_text = help_text;
      return attribute_spec;
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
   std::vector<ConfigAttributeSpecification> attribute_specifications;

   [[nodiscard]] std::string helpText() const;

   [[nodiscard]] std::optional<ConfigAttributeSpecification> getAttributeSpecification(
      const ConfigKeyPath& key
   ) const;

   [[nodiscard]] std::optional<ConfigAttributeSpecification>
   getAttributeSpecificationFromAmbiguousKey(const AmbiguousConfigKeyPath& key) const;

   /// Convert the ConfigSpecification to a VerifiedConfigAttributes, to
   /// use as the source for the default values.
   [[nodiscard]] VerifiedConfigAttributes getConfigSourceFromDefaults() const;
};

}  // namespace silo::config
