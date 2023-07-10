#include "silo/preprocessing/metadata_validator.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <string>
#include <vector>

#include <csv.hpp>

#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

void MetadataValidator::validateMedataFile(
   const std::filesystem::path& metadata_file,
   const silo::config::DatabaseConfig& database_config
) const {
   const auto metadata_reader = MetadataReader(metadata_file);
   const auto metadata_columns = metadata_reader.reader.get_col_names();

   for (const auto& config_column : database_config.schema.metadata) {
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
      const auto corresponding_entry_in_database_config = std::find_if(
         database_config.schema.metadata.begin(),
         database_config.schema.metadata.end(),
         [&metadata_column](const auto& config_column) {
            return config_column.name == metadata_column;
         }
      );
      if (corresponding_entry_in_database_config == database_config.schema.metadata.end()) {
         SPDLOG_WARN(
            "Metadata file contains a column that is not in the config: {}. It will not be used.",
            metadata_column
         );
      }
   }
}

}  // namespace silo::preprocessing
