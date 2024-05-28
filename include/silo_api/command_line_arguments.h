#pragma once

#include <Poco/Util/AbstractConfiguration.h>

#include "silo/config/util/abstract_config_source.h"

namespace silo_api {

class CommandLineArguments : public silo::config::AbstractConfigSource {
   const Poco::Util::AbstractConfiguration& config;

  public:
   static std::string asUnixOptionString(const Option& option);

   explicit CommandLineArguments(const Poco::Util::AbstractConfiguration& config);

   [[nodiscard]] std::string configType() const override;

   [[nodiscard]] bool hasProperty(const Option& option) const override;

   [[nodiscard]] std::optional<std::string> getString(const Option& option) const override;
};

}  // namespace silo_api
