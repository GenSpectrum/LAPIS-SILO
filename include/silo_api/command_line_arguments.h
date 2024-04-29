#pragma once

#include <Poco/Util/AbstractConfiguration.h>

#include "silo/config/util/abstract_config.h"

namespace silo_api {

class CommandLineArguments : public silo::config::AbstractConfig {
   const Poco::Util::AbstractConfiguration& config;

  public:
   explicit CommandLineArguments(const Poco::Util::AbstractConfiguration& config);

   std::string configType() const override;

   bool hasProperty(const std::string& key) const override;

   std::string getString(const std::string& key) const override;

   int32_t getInt32(const std::string& key) const override;

   uint32_t getUInt32(const std::string& key) const override;
};

}  // namespace silo_api
