#pragma once

#include <string_view>

namespace silo::config {

class AbstractConfig {
  public:
   virtual std::string configType() const = 0;
   virtual bool hasProperty(const std::string& key) const = 0;
   virtual std::string getString(const std::string& key) const = 0;
   virtual int32_t getInt32(const std::string& key) const = 0;
   virtual uint32_t getUInt32(const std::string& key) const = 0;
};

}  // namespace silo::config
