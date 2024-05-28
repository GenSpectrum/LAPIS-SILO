#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace silo::config {

class AbstractConfigSource {
  public:
   class Option {
     public:
      std::vector<std::string> access_path;

      [[nodiscard]] std::string toString() const;
      [[nodiscard]] std::string toCamelCase() const;
   };

   [[nodiscard]] virtual std::string configType() const = 0;

   [[nodiscard]] virtual bool hasProperty(const Option& option) const = 0;
   [[nodiscard]] virtual std::optional<std::string> getString(const Option& option) const = 0;
   [[nodiscard]] virtual std::optional<int32_t> getInt32(const Option& option) const;
   [[nodiscard]] virtual std::optional<uint32_t> getUInt32(const Option& option) const;
};

}  // namespace silo::config
