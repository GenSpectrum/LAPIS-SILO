#pragma once

#include <filesystem>

#include "silo/config/util/abstract_config_source.h"

namespace silo_api {

class EnvironmentVariables : public silo::config::AbstractConfigSource {
  public:
   static std::string prefixedUppercase(const Option& option);

   std::string configType() const override;

   bool hasProperty(const Option& option) const override;

   std::optional<std::string> getString(const Option& option) const override;
};

}  // namespace silo_api
