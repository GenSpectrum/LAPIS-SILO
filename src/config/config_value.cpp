#include "config/config_value.h"

#include <fmt/ranges.h>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "silo/common/panic.h"

namespace silo::config {

ConfigValueType ConfigValue::getValueType() const {
   if (std::holds_alternative<std::string>(value)) {
      return ConfigValueType::STRING;
   }
   if (std::holds_alternative<std::filesystem::path>(value)) {
      return ConfigValueType::PATH;
   }
   if (std::holds_alternative<int32_t>(value)) {
      return ConfigValueType::INT32;
   }
   if (std::holds_alternative<uint32_t>(value)) {
      return ConfigValueType::UINT32;
   }
   if (std::holds_alternative<uint16_t>(value)) {
      return ConfigValueType::UINT16;
   }
   if (std::holds_alternative<bool>(value)) {
      return ConfigValueType::BOOL;
   }
   if (std::holds_alternative<std::vector<std::string>>(value)) {
      return ConfigValueType::LIST;
   }
   SILO_UNREACHABLE();
}

std::string ConfigValue::toString() const {
   return std::visit(
      [](const auto& value) -> std::string {
         using T = std::decay_t<decltype(value)>;
         if constexpr (std::is_same_v<T, std::string>) {
            return fmt::format("'{}'", value);
         } else if constexpr (std::is_same_v<T, std::filesystem::path>) {
            return fmt::format("'{}'", value.string());
         } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            return fmt::format("{}", fmt::join(value, ","));
         } else {
            return fmt::format("{}", value);
         }
      },
      value
   );
}

}  // namespace silo::config
