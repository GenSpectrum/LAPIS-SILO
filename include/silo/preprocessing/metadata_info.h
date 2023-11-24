#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "silo/config/database_config.h"

namespace silo {

namespace preprocessing {

class PreprocessingDatabase;

class MetadataInfo {
   silo::config::DatabaseConfig database_config;
   std::unordered_map<std::string, std::string> metadata_selects;

   MetadataInfo() = default;

  public:
   static MetadataInfo validateFromMetadataFile(
      const std::filesystem::path& metadata_file,
      const silo::config::DatabaseConfig& database_config
   );

   static MetadataInfo validateFromNdjsonFile(
      const std::filesystem::path& ndjson_file,
      const silo::config::DatabaseConfig& database_config
   );

   std::vector<std::string> getMetadataFields() const;

   std::vector<std::string> getMetadataSelects() const;
};
}  // namespace preprocessing

}  // namespace silo
