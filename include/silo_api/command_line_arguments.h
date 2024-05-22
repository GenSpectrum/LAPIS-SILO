#pragma once

#include <Poco/Util/AbstractConfiguration.h>

#include "silo/config/util/abstract_config_source.h"

namespace silo_api {

class CommandLineArguments : public silo::config::AbstractConfigSource {
   const Poco::Util::AbstractConfiguration& config;

  public:
   static std::string asUnixOptionString(const Option& option);

   explicit CommandLineArguments(const Poco::Util::AbstractConfiguration& config);

   std::string configType() const override;

   bool hasProperty(const Option& option) const override;

   std::optional<std::string> getString(const Option& option) const override;
};

}  // namespace silo_api
