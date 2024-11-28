#pragma once

#include <unistd.h>

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "config/config_source_interface.h"
#include "silo/config/util/config_exception.h"

#ifdef __APPLE__
// macOS-specific
extern const char* const* environ;
#endif

namespace silo::config {

// (Inheriting implementation for ConfigSource directly.)
class EnvironmentVariables : public ConfigSource {
   /* EnvironmentVariables base, */
   std::vector<std::pair<std::string, std::string>> alist;

   explicit EnvironmentVariables(std::vector<std::pair<std::string, std::string>>&& alist_)
       : alist(std::move(alist_)){};

   explicit EnvironmentVariables(){};

  public:
   [[nodiscard]] VerifiedConfigSource verify(const ConfigSpecification& config_specification
   ) const override;

   static EnvironmentVariables decodeEnvironmentVariables(const char* const* envp = environ);

   [[nodiscard]] constexpr std::string_view errorContext() const {
      return "environment variables";
   };

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static AmbiguousConfigKeyPath stringToConfigKeyPath(const std::string& key_path_string);
};

}  // namespace silo::config
