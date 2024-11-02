#pragma once

#include <string>
#include <utility>
#include <vector>

#include "config/config_source_interface.h"
#include "silo/config/util/config_exception.h"

class DecodedEnvironmentVariables;

class EnvironmentVariables : public ConfigSource {
  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   /// Retrieve environment variables, return the relevant set
   /// ("SILO_"-prefixed) with their values in (unicode-decoded form).
   static std::unique_ptr<DecodedEnvironmentVariables> parse(const char* const* envp = environ);
};

// (Inheriting implementation for ConfigSource directly.)
class DecodedEnvironmentVariables : public VerifyConfigSource {
   /* EnvironmentVariables base, */
   std::vector<std::pair<std::string, std::string>> alist;

   explicit DecodedEnvironmentVariables(std::vector<std::pair<std::string, std::string>>&& alist_)
       : alist(std::move(alist_)){};

   friend class EnvironmentVariables;
   friend class VerifiedEnvironmentVariables;

  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::unique_ptr<VerifiedConfigSource> verify(
      const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
   ) override;
};

class VerifiedEnvironmentVariables : public VerifiedConfigSource {
   DecodedEnvironmentVariables base;

   friend class DecodedEnvironmentVariables;
   explicit VerifiedEnvironmentVariables(DecodedEnvironmentVariables&& base_)
       : base(std::move(base_)) {}

  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& config_key_path
   ) const override;
   [[nodiscard]] const std::vector<std::string>* positionalArgs() const override;
};
