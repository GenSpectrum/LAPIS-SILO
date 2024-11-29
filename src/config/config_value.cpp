#include "config/config_value.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "config/config_exception.h"
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

}  // namespace silo::config
