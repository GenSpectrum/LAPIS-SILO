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

      std::string toString() const;
      std::string toCamelCase() const;
   };

   virtual std::string configType() const = 0;

   virtual bool hasProperty(const Option& option) const = 0;
   virtual std::optional<std::string> getString(const Option& option) const = 0;
   virtual std::optional<int32_t> getInt32(const Option& option) const;
   virtual std::optional<uint32_t> getUInt32(const Option& option) const;
};

}  // namespace silo::config
