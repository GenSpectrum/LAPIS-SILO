#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "silo/config/database_config.h"
#include "silo/preprocessing/identifiers.h"

namespace silo::preprocessing {

class MetadataInfo {
  public:
   static void validateMetadataFile(
      const std::filesystem::path& metadata_file,
      const silo::config::DatabaseConfig& database_config
   );

   static bool isNdjsonFileEmpty(const std::filesystem::path& ndjson_file);
   static void validateNdjsonFile(
      const std::filesystem::path& ndjson_file,
      const silo::config::DatabaseConfig& database_config
   );

   static silo::preprocessing::Identifiers getMetadataFields(
      const silo::config::DatabaseConfig& database_config
   );

   static std::vector<std::string> getMetadataSQLTypes(
      const silo::config::DatabaseConfig& database_config
   );

   static std::vector<std::string> getMetadataSelects(
      const silo::config::DatabaseConfig& database_config
   );
};

}  // namespace silo::preprocessing
