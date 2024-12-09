#pragma once

#include <unistd.h>

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "config/config_exception.h"
#include "config/config_source_interface.h"

#ifdef __APPLE__
// macOS-specific
extern const char* const* environ;
#endif

namespace silo::config {

class EnvironmentVariables {
   std::vector<std::pair<std::string, std::string>> association_list;
   std::vector<std::string> allow_list;

   explicit EnvironmentVariables(
      std::vector<std::pair<std::string, std::string>>&& association_list_,
      std::vector<std::string> allow_list_
   )
       : association_list(std::move(association_list_)),
         allow_list(std::move(allow_list_)){};

   explicit EnvironmentVariables() = default;

   [[nodiscard]] std::string debugContext() const { return "environment variables"; };

  public:
   static EnvironmentVariables newWithAllowListAndEnv(
      const std::vector<std::string>& allow_list,
      const char* const* envp
   );

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static AmbiguousConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);

   using VerifiedType = VerifiedConfigAttributes;
   [[nodiscard]] VerifiedType verify(const ConfigSpecification& config_specification) const;
};

}  // namespace silo::config
