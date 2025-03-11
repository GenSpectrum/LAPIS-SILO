#include "silo/common/silo_directory.h"

#include <spdlog/spdlog.h>

namespace silo {

std::optional<SiloDataSource> SiloDataSource::checkValidDataSource(
   const std::filesystem::path& candidate_data_source_path
) {
   if (!std::filesystem::is_directory(candidate_data_source_path)) {
      SPDLOG_TRACE("Skipping {} because it is not a directory", candidate_data_source_path);
      return std::nullopt;
   }
   auto folder_name_timestamp =
      silo::DataVersion::Timestamp::fromString(candidate_data_source_path.filename());
   if (folder_name_timestamp == std::nullopt) {
      SPDLOG_TRACE(
         "Skipping {}. Its name is not a valid data version.", candidate_data_source_path.string()
      );
      return std::nullopt;
   }
   auto data_version_filename = candidate_data_source_path / "data_version.silo";
   if (!std::filesystem::is_regular_file(data_version_filename)) {
      SPDLOG_TRACE(
         "Skipping {}. it does not contain the data version file {}, which "
         "confirms a finished and valid data source",
         candidate_data_source_path.string(),
         data_version_filename.string()
      );
      return std::nullopt;
   }
   auto maybe_data_version_in_file = silo::DataVersion::fromFile(data_version_filename);
   if (maybe_data_version_in_file == std::nullopt) {
      SPDLOG_TRACE(
         "Skipping {}. The data version in data_version.silo could not be parsed",
         candidate_data_source_path.string()
      );
      return std::nullopt;
   }
   auto data_version_in_file = maybe_data_version_in_file.value();

   if (data_version_in_file.getTimestamp() != folder_name_timestamp) {
      SPDLOG_WARN(
         "Skipping {}. The data version in data_version.silo is not equal to the directory name",
         candidate_data_source_path.string()
      );
      return std::nullopt;
   }
   SiloDataSource data_source{candidate_data_source_path, data_version_in_file};
   return data_source;
}

std::optional<SiloDataSource> SiloDirectory::getMostRecentDataDirectory() const {
   SPDLOG_TRACE("Scanning path {} for valid data", directory.string());
   std::vector<SiloDataSource> all_found_data;
   for (const auto& directory_entry : std::filesystem::directory_iterator{directory}) {
      SPDLOG_TRACE("Checking directory entry {}", directory_entry.path().string());
      auto silo_data_source = SiloDataSource::checkValidDataSource(directory_entry.path());
      if (silo_data_source.has_value()) {
         SPDLOG_TRACE(
            "Found candidate data source {} with data version {}",
            directory_entry.path().string(),
            silo_data_source.value().data_version.toString()
         );
         all_found_data.emplace_back(std::move(silo_data_source.value()));
      }
   }
   if (all_found_data.empty()) {
      SPDLOG_TRACE("Scan of path {} returned no valid data", directory.string());
      return std::nullopt;
   }
   std::ranges::sort(all_found_data, [](const auto& element1, const auto& element2) {
      return element1.data_version > element2.data_version;
   });
   for (auto& entry : all_found_data) {
      if (entry.data_version.isCompatibleVersion()) {
         SPDLOG_TRACE(
            "Selected newest data which is compatible ({}) in path {} for ingestion",
            entry.data_version.toString(),
            entry.path.string()
         );
         return entry;
      }
      SPDLOG_WARN(
         "The database output {} is incompatible with the current SILO version {}.",
         entry.data_version.toString(),
         silo::DataVersion::CURRENT_SILO_SERIALIZATION_VERSION.value
      );
   }
   return std::nullopt;
}

}  // namespace silo
