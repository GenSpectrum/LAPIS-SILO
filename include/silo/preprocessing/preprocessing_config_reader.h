#ifndef SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_
#define SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_

#include <filesystem>

namespace silo::preprocessing {
struct PreprocessingConfig;

class PreprocessingConfigReader {
  public:
   virtual PreprocessingConfig readConfig(const std::filesystem::path& config_path) const;
};
}  // namespace silo::preprocessing

#endif  // SILO_INCLUDE_SILO_API_PREPROCESSING_CONFIG_READER_H_
