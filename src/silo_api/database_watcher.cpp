#include "silo_api/database_watcher.h"

#include <ranges>

#include <boost/algorithm/string/join.hpp>

silo_api::DatabaseDirectoryWatcher::DatabaseDirectoryWatcher(
   const std::filesystem::path& path,
   DatabaseMutex& database_mutex
)
    : path(path),
      database_mutex(database_mutex),
      timer(0, 2000) {
   timer.start(Poco::TimerCallback<DatabaseDirectoryWatcher>(
      *this, &DatabaseDirectoryWatcher::checkDirectoryForData
   ));
}

namespace {

std::optional<silo::DataVersion> checkValidDataSource(const std::filesystem::path& path) {
   if (!std::filesystem::is_directory(path)) {
      SPDLOG_INFO("Skipping {} because it is not a directory", path.string());
      return std::nullopt;
   }
   auto data_version = silo::DataVersion::fromString(path.filename().string());
   if (data_version == std::nullopt) {
      SPDLOG_INFO("Skipping {}. Its name is not a valid data version.", path.string());
      return std::nullopt;
   }
   auto data_version_filename = path / "data_version.silo";
   if (!std::filesystem::is_regular_file(data_version_filename)) {
      SPDLOG_INFO(
         "Skipping {}. it does not contain the data version file {}, which "
         "confirms a finished and valid data source",
         path.string(),
         data_version_filename.string()
      );
      return std::nullopt;
   }
   std::ifstream data_version_file(data_version_filename);
   if (!data_version_file) {
      SPDLOG_INFO("Skipping {}. The data version file could not be opened", path.string());
      return std::nullopt;
   }
   std::string data_version_in_file_string;
   data_version_file >> data_version_in_file_string;
   const auto data_version_in_file = silo::DataVersion::fromString(data_version_in_file_string);
   if (data_version_in_file == std::nullopt) {
      SPDLOG_INFO(
         "Skipping {}. The data version in data_version.silo could not be parsed", path.string()
      );
      return std::nullopt;
   }
   if (data_version_in_file != data_version) {
      SPDLOG_INFO(
         "Skipping {}. The data version in data_version.silo is not equal to the directory name",
         path.string()
      );
      return std::nullopt;
   }
   return data_version.value();
}

std::optional<std::pair<std::filesystem::path, silo::DataVersion>> getMostRecentDataDirectory(
   std::filesystem::path path
) {
   SPDLOG_INFO("Scanning path {} for valid data", path.string());
   std::vector<std::pair<std::filesystem::path, silo::DataVersion>> all_found_data;
   for (const auto& directory_entry : std::filesystem::directory_iterator{path}) {
      SPDLOG_INFO("Checking directory entry {}", directory_entry.path().string());
      auto data_version = checkValidDataSource(directory_entry.path());
      if (data_version.has_value()) {
         SPDLOG_INFO(
            "Found candidate data source {} with data version {}",
            directory_entry.path().string(),
            data_version->toString()
         );
         all_found_data.emplace_back(directory_entry.path(), std::move(*data_version));
      }
   }
   if (all_found_data.empty()) {
      SPDLOG_INFO("No previous data found, place data in {} for ingestion", path.string());
      return std::nullopt;
   }
   if (all_found_data.size() == 1) {
      const auto& found_data = all_found_data.at(0);
      SPDLOG_INFO("Using previous data: {}", found_data.first.string());
      return found_data;
   }
   auto max_element = std::max_element(
      all_found_data.begin(),
      all_found_data.end(),
      [](const auto& element1, const auto& element2) { return element1.second < element2.second; }
   );
   SPDLOG_INFO(
      "Selected database with highest data-version {} in path {} for ingestion",
      max_element->second.toString(),
      max_element->first.string()
   );
   return *max_element;
}

}  // namespace

void silo_api::DatabaseDirectoryWatcher::checkDirectoryForData(Poco::Timer& /*timer*/) {
   auto most_recent_database_state = getMostRecentDataDirectory(path);

   {
      const auto fixed_database = database_mutex.getDatabase();
      if (fixed_database.database.getDataVersion() >= most_recent_database_state->second) {
         SPDLOG_INFO(
            "Do not update data version to {} in path {}. Its version is not newer than the "
            "current version",
            most_recent_database_state->second.toString(),
            most_recent_database_state->first.string()
         );
         timer.restart();
         return;
      }
   }

   SPDLOG_INFO("New data version detected: {}", most_recent_database_state->first.string());
   database_mutex.setDatabase(silo::Database::loadDatabaseState(most_recent_database_state->first));
   timer.restart();
}