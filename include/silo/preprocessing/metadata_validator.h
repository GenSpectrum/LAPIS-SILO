#ifndef SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_
#define SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_

#include <filesystem>
#include "silo/config/config_repository.h"
#include "silo/preprocessing/metadata.h"

namespace silo::preprocessing {

class MetadataValidator {
  public:
   explicit MetadataValidator(
      const config::ConfigRepository& config_repository = config::ConfigRepository()
   );

   virtual void validateMedataFile(
      const std::filesystem::path& metadata_file,
      const std::filesystem::path& config_file
   ) const;

  private:
   const config::ConfigRepository& config_repository_;
};

}  // namespace silo::preprocessing

#endif  // SILO_INCLUDE_SILO_PREPROCESSING_METADATA_VALIDATOR_H_
