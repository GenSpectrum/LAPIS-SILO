#ifndef SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_
#define SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_

#include <filesystem>

#include "silo/config/config_repository.h"
#include "silo/preprocessing/metadata.h"

namespace silo::config {
struct DatabaseConfig;
}  // namespace silo::config

namespace silo::preprocessing {

class MetadataValidator {
  public:
   virtual void validateMedataFile(
      const std::filesystem::path& metadata_file,
      const silo::config::DatabaseConfig& database_config
   ) const;
};

}  // namespace silo::preprocessing

#endif  // SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_
