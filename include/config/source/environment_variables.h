#pragma once

#include <unistd.h>

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "config/config_exception.h"
#include "config/config_source_interface.h"
#include "config/config_specification.h"

#ifdef __APPLE__
// macOS-specific
extern const char* const* environ;
#endif

namespace silo::config {

class EnvironmentVariables {
   std::vector<std::pair<std::string, std::string>> key_value_pairs;
   std::vector<std::string> allow_list;

   explicit EnvironmentVariables(
      std::vector<std::pair<std::string, std::string>>&& key_value_pairs_,
      std::vector<std::string> allow_list_
   )
       : key_value_pairs(std::move(key_value_pairs_)),
         allow_list(std::move(allow_list_)){};

   explicit EnvironmentVariables() = default;

  public:
   static EnvironmentVariables newWithAllowListAndEnv(
      const std::vector<std::string>& allow_list,
      const char* const* envp
   );

   [[nodiscard]] inline std::string debugContext() const { return "environment variables"; };

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static AmbiguousConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);

   using VerifiedType = VerifiedConfigAttributes;
   [[nodiscard]] VerifiedType verify(const ConfigSpecification& config_specification) const;
};
static_assert(
   ConfigSource<EnvironmentVariables>,
   "EnvironmentVariables does not satisfy the concept ConfigSource"
);

}  // namespace silo::config
