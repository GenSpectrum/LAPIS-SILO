#pragma once

//! Part of config metadata, but can't be in `config_metadata.h` due to
//! that depending on `config/config_source_interface.h` which also
//! references `ConfigValue`.

#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "config/config_key_path.h"
#include "silo/common/panic.h"

namespace silo::config {

enum class ConfigValueType { STRING, PATH, INT32, UINT32, UINT16, BOOL };

constexpr std::string_view configValueTypeToString(ConfigValueType type) {
   switch (type) {
      case ConfigValueType::STRING:
         return "string";
      case ConfigValueType::PATH:
         return "path";
      case ConfigValueType::INT32:
         return "i32";
      case ConfigValueType::UINT32:
         return "u32";
      case ConfigValueType::UINT16:
         return "u16";
      case ConfigValueType::BOOL:
         return "bool";
   }
   SILO_UNREACHABLE();
}

// Forward declaration for friend class access
class ConfigValueSpecification;

class ConfigValue {
   friend class ConfigValueSpecification;

   ConfigValue(
      std::variant<std::string, std::filesystem::path, int32_t, uint32_t, uint16_t, bool> value
   )
       : value(value) {}

  public:
   std::variant<std::string, std::filesystem::path, int32_t, uint32_t, uint16_t, bool> value;

   static ConfigValue fromString(const std::string& value) { return ConfigValue{value}; }

   static ConfigValue fromPath(const std::filesystem::path& value) {
      const ConfigValue result{value};
      SILO_ASSERT(get_if<std::filesystem::path>(&result.value) != nullptr);
      return result;
   }

   static ConfigValue fromInt32(int32_t value) { return ConfigValue{value}; }

   static ConfigValue fromUint32(uint32_t value) { return ConfigValue{value}; }

   static ConfigValue fromUint16(uint16_t value) { return ConfigValue{value}; }

   static ConfigValue fromBool(bool value) { return ConfigValue{value}; }

   ConfigValueType getValueType() const;

   std::string toString() const;
};

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

   ConfigValue getValueFromString(std::string value_string) const;

   ConfigValue createValue(
      std::variant<std::string, std::filesystem::path, int32_t, uint32_t, uint16_t, bool> value
   ) const;

   static ConfigValueSpecification createWithoutDefault(
      ConfigKeyPath key,
      ConfigValueType value_type,
      std::string_view help_text
   ) {
      ConfigValueSpecification value_specification;
      value_specification.key = key;
      value_specification.type = value_type;
      value_specification.help_text = help_text;
      return value_specification;
   }

   /// No need for the value_type. It is implicitly defined by the default. Prevents
   /// misspecification.
   static ConfigValueSpecification createWithDefault(
      ConfigKeyPath key,
      ConfigValue default_value,
      std::string_view help_text
   ) {
      ConfigValueSpecification value_specification;
      value_specification.key = key;
      value_specification.type = default_value.getValueType();
      value_specification.default_value = default_value;
      value_specification.help_text = help_text;
      return value_specification;
   }
};

}  // namespace silo::config
