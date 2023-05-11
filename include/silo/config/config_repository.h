#ifndef SILO_INCLUDE_SILO_CONFIG_CONFIG_REPOSITORY_H_
#define SILO_INCLUDE_SILO_CONFIG_CONFIG_REPOSITORY_H_

#include <filesystem>

#include "silo/config/database_config.h"
#include "silo/config/database_config_reader.h"

namespace silo::config {

class ConfigRepository {
  public:
   explicit ConfigRepository(const DatabaseConfigReader& reader = DatabaseConfigReader());

   virtual DatabaseConfig getValidatedConfig(const std::filesystem::path& path) const;
   virtual std::string getPrimaryKey(const std::filesystem::path& path) const;
   virtual DatabaseMetadata getMetadata(const std::filesystem::path& path, const std::string& name)
      const;

  private:
   const DatabaseConfigReader& reader_;
   virtual void validateConfig(const DatabaseConfig& config) const;
};

}  // namespace silo::config

#endif  // SILO_INCLUDE_SILO_CONFIG_CONFIG_REPOSITORY_H_
