#pragma once

//! Structs with which to declare metainformation on structs that are
//! to hold configuration data.

#include <optional>
#include <span>
#include <variant>

#include <config/config_key_path.h>
#include <config/config_source_interface.h>
#include <silo/common/cons_list.h>
#include "config/config_value.h"

// Using `const char*` for string literals; this makes those easier to
// use (std::string_view isn't compatible to the same level).

class ConfigStruct;

using ConfigValueOrStruct = std::variant<ConfigValue, const ConfigStruct*>;

// This carries everything except (actual struct field name and)
// accessor generation.
class ConfigStructField {
  public:
   /// Config key string (usually the same as the struct field name),
   /// in camelCase.
   const char* field_name_camel;
   /// Either a value, or an inner struct.
   ConfigValueOrStruct value;
};

/// Does not support extracting non-option arguments; those wouldn't
/// be supported by env vars or config files anyway, although could
/// still be specified for command line, but that's not implemented
/// currently.
class ConfigStruct : public VerifiedConfigSource {
  public:
   const char* program_or_struct_name;
   // Using std::vector so that initialization in place is possible;
   // std::span would require the array to exist in a different global
   // first, don't want to make it verbose like that. Paying with
   // dropping constexpr for that.
   std::vector<ConfigStructField> fields;

   ConfigStruct(const char* program_or_struct_name, std::vector<ConfigStructField> fields)
       : program_or_struct_name(program_or_struct_name),
         fields(std::move(fields)) {}

   // Does not check for duplicates! Use `config_values` instead.
   void collectConfigValues(
      const ConsList<std::string>& parents,
      std::vector<std::pair<ConfigKeyPath, const ConfigValue*>>& fields
   ) const;

   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& config_key_path
   ) const override;
   [[nodiscard]] const std::vector<std::string>* positionalArgs() const override;

   [[nodiscard]] std::vector<std::pair<ConfigKeyPath, const ConfigValue*>> configValues() const;

   [[nodiscard]] std::string helpText() const;
};
