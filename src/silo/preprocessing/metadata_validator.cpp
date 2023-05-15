#include "silo/preprocessing/metadata_validator.h"

#include <spdlog/spdlog.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

MetadataValidator::MetadataValidator(const config::ConfigRepository& config_repository)
    : config_repository_(config_repository) {}

void MetadataValidator::validateMedataFile(
   const std::filesystem::path& metadata_file,
   const std::filesystem::path& config_file
) const {
   const auto csv_reader = MetadataReader::getReader(metadata_file);
   const auto config = config_repository_.getValidatedConfig(config_file);
   const auto metadata_columns = csv_reader.get_col_names();

   for (const auto& config_column : config.schema.metadata) {
      if (std::find(
           metadata_columns.begin(),
           metadata_columns.end(),
           config_column.name
        ) == metadata_columns.end()) {
         throw PreprocessingException(
            "Metadata file does not contain column: " + config_column.name
         );
      }
   }

   for (const auto& metadata_column : metadata_columns) {
      if (std::find_if(config.schema.metadata.begin(), config.schema.metadata.end(), [&metadata_column](const auto& config_column) {
             return config_column.name == metadata_column;
          }) == config.schema.metadata.end()) {
         SPDLOG_WARN(
            "Metadata file contains a column that is not in the config: {}. It will not be used.",
            metadata_column
         );
      }
   }
}

}  // namespace silo::preprocessing