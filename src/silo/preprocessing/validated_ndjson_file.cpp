#include "silo/preprocessing/validated_ndjson_file.h"

#include <spdlog/spdlog.h>

#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sequence_info.h"
#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

ValidatedNdjsonFile::ValidatedNdjsonFile(std::filesystem::path file_name, bool empty)
    : file_name(std::move(file_name)),
      empty(empty) {}

std::filesystem::path ValidatedNdjsonFile::getFileName() const {
   return file_name;
}

bool ValidatedNdjsonFile::isEmpty() const {
   return empty;
}

ValidatedNdjsonFile ValidatedNdjsonFile::validateFileAgainstConfig(
   const std::filesystem::path& file_name,
   const silo::config::DatabaseConfig& database_config,
   const silo::ReferenceGenomes& reference_genomes
) {
   SPDLOG_DEBUG("build - checking whether the file '{}' exists: ", file_name.string());
   if (!std::filesystem::exists(file_name)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The specified input file {} does not exist.", file_name.string())
      );
   }

   SPDLOG_DEBUG("build - checking whether the file '{}' is not a directory: ", file_name.string());
   if (std::filesystem::is_directory(file_name)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The specified input file {} is a directory.", file_name.string())
      );
   }

   const bool empty = MetadataInfo::isNdjsonFileEmpty(file_name);

   SPDLOG_DEBUG("build - checking whether the file '{}' is empty: ", file_name.string());

   if (!empty) {
      SPDLOG_DEBUG("build - validating metadata file '{}' with config", file_name.string());
      MetadataInfo::validateNdjsonFile(file_name, database_config);

      SPDLOG_DEBUG("build - validating metadata file '{}' with config", file_name.string());
      SequenceInfo::validateNdjsonFile(reference_genomes, file_name);
   }

   return ValidatedNdjsonFile(file_name, empty);
}
}  // namespace silo::preprocessing
