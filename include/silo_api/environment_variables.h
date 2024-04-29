#pragma once

#include <filesystem>

#include "silo/config/util/abstract_config.h"

namespace silo_api {

class EnvironmentVariables : public silo::config::AbstractConfig {
  public:
   static std::string prefixedUppercase(const std::string& key);

   std::string configType() const override;

   bool hasProperty(const std::string& key) const override;

   std::string getString(const std::string& key) const override;

   int32_t getInt32(const std::string& key) const override;

   uint32_t getUInt32(const std::string& key) const override;
};

}  // namespace silo_api
