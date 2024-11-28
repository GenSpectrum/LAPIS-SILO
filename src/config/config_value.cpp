#include "config/config_value.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "config/source/yaml_file.h"
#include "silo/common/panic.h"

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
   switch (type) {
      case ConfigValueType::STRING:
         return createValue(value_string);
      case ConfigValueType::PATH: {
         std::filesystem::path path = value_string;
         return createValue(path);
      }
      case ConfigValueType::UINT32: {
         const auto parsed_unsigned = boost::lexical_cast<uint32_t>(value_string);
         return createValue(parsed_unsigned);
      }
      case ConfigValueType::UINT16: {
         const auto parsed_unsigned = boost::lexical_cast<uint16_t>(value_string);
         return createValue(parsed_unsigned);
      }
      case ConfigValueType::INT32: {
         const auto parsed_signed = boost::lexical_cast<int32_t>(value_string);
         return createValue(parsed_signed);
      }
      case ConfigValueType::BOOL:
         return createValue(true);
   }
   SILO_UNREACHABLE();
}

ConfigValue ConfigValueSpecification::createValue(
   std::variant<std::string, std::filesystem::path, int32_t, uint32_t, uint16_t, bool> value
) const {
   ConfigValue created_value{std::move(value)};
   if (created_value.getValueType() != type) {
      throw std::runtime_error(
         "Internal Error: value created for this specification that is of the wrong type."
      );
   }
   return created_value;
}

}  // namespace silo::config
