#pragma once

#include <filesystem>

#include "silo/config/database_config.h"
#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

class ValidatedNdjsonFile {
   std::filesystem::path file_name;
   bool empty;

   explicit ValidatedNdjsonFile(std::filesystem::path file_name, bool empty);

  public:
   std::filesystem::path getFileName() const;

   bool isEmpty() const;

   static ValidatedNdjsonFile validateFileAgainstConfig(
      const std::filesystem::path& file_name,
      const silo::config::DatabaseConfig& database_config,
      const silo::ReferenceGenomes& reference_genomes
   );
};

}  // namespace silo::preprocessing
