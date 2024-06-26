#pragma once

#include <filesystem>

#include "silo/config/util/abstract_config_source.h"

namespace silo_api {

class EnvironmentVariables : public silo::config::AbstractConfigSource {
  public:
   static std::string prefixedUppercase(const Option& option);

   [[nodiscard]] std::string configType() const override;

   [[nodiscard]] bool hasProperty(const Option& option) const override;

   [[nodiscard]] std::optional<std::string> getString(const Option& option) const override;
};

}  // namespace silo_api
