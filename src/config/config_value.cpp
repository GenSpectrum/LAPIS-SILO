#include "config/config_value.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "config/source/yaml_file.h"
#include "silo/common/panic.h"
#include "silo/config/util/config_exception.h"

namespace silo::config {

ConfigValueType ConfigValue::getValueType() const {
   return std::visit(
      [](const auto& value) -> ConfigValueType {
         using T = std::decay_t<decltype(value)>;
         if constexpr (std::is_same_v<T, std::string>) {
            return ConfigValueType::STRING;
         } else if constexpr (std::is_same_v<T, std::filesystem::path>) {
            return ConfigValueType::PATH;
         } else if constexpr (std::is_same_v<T, int32_t>) {
            return ConfigValueType::INT32;
         } else if constexpr (std::is_same_v<T, uint32_t>) {
            return ConfigValueType::UINT32;
         } else if constexpr (std::is_same_v<T, uint16_t>) {
            return ConfigValueType::UINT16;
         } else if constexpr (std::is_same_v<T, bool>) {
            return ConfigValueType::BOOL;
         } else {
            SILO_UNREACHABLE();
         }
      },
      value
   );
}

std::string ConfigValue::toString() const {
   return std::visit(
      [](const auto& value) -> std::string {
         using T = std::decay_t<decltype(value)>;
         if constexpr (std::is_same_v<T, std::string>) {
            return fmt::format("'{}'", value);
         } else if constexpr (std::is_same_v<T, std::filesystem::path>) {
            return fmt::format("'{}'", value.string());
         } else {
            return fmt::format("{}", value);
         }
      },
      value
   );
}

ConfigValue ConfigValueSpecification::getValueFromString(std::string value_string) const {
   try {
      switch (type) {
         case ConfigValueType::STRING:
            return ConfigValue::fromString(value_string);
         case ConfigValueType::PATH: {
            return ConfigValue::fromPath(value_string);
         }
         case ConfigValueType::UINT32: {
            // Because boost does not error on negative numbers
            if (value_string.starts_with('-')) {
               throw ConfigException(fmt::format(
                  "cannot parse negative number '{}' as unsigned type {}",
                  value_string,
                  configValueTypeToString(type)
               ));
            }
            const auto parsed_unsigned = boost::lexical_cast<uint32_t>(value_string);
            return ConfigValue::fromUint32(parsed_unsigned);
         }
         case ConfigValueType::UINT16: {
            // Because boost does not error on negative numbers
            if (value_string.starts_with('-')) {
               throw ConfigException(fmt::format(
                  "cannot parse negative number '{}' as unsigned type {}",
                  value_string,
                  configValueTypeToString(type)
               ));
            }
            const auto parsed_unsigned = boost::lexical_cast<uint16_t>(value_string);
            return ConfigValue::fromUint16(parsed_unsigned);
         }
         case ConfigValueType::INT32: {
            const auto parsed_signed = boost::lexical_cast<int32_t>(value_string);
            return ConfigValue::fromInt32(parsed_signed);
         }
         case ConfigValueType::BOOL:
            if (value_string == "true" || value_string == "1") {
               return ConfigValue::fromBool(true);
            }
            if (value_string == "false" || value_string == "0") {
               return ConfigValue::fromBool(false);
            }
      }
      SILO_UNREACHABLE();
   } catch (boost::bad_lexical_cast& _) {
      throw ConfigException(
         fmt::format("cannot parse '{}' as {}", value_string, configValueTypeToString(type))
      );
   }
}

}  // namespace silo::config
