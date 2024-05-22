#include "silo_api/database_directory_watcher.h"

#include <cxxabi.h>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "silo/common/data_version.h"
#include "silo/database.h"
#include "silo_api/database_mutex.h"

silo_api::DatabaseDirectoryWatcher::DatabaseDirectoryWatcher(
   std::filesystem::path path,
   DatabaseMutex& database_mutex
)
    : path(std::move(path)),
      database_mutex(database_mutex),
      timer(0, 2000) {
   timer.start(Poco::TimerCallback<DatabaseDirectoryWatcher>(
      *this, &DatabaseDirectoryWatcher::checkDirectoryForData
   ));
}

std::optional<silo::DataVersion> silo_api::DatabaseDirectoryWatcher::checkValidDataSource(
   const std::filesystem::path& path
) {
   if (!std::filesystem::is_directory(path)) {
      SPDLOG_TRACE("Skipping {} because it is not a directory", path.string());
      return std::nullopt;
   }
   auto folder_name_timestamp = silo::DataVersion::Timestamp::fromString(path.filename().string());
   if (folder_name_timestamp == std::nullopt) {
      SPDLOG_TRACE("Skipping {}. Its name is not a valid data version.", path.string());
      return std::nullopt;
   }
   auto data_version_filename = path / "data_version.silo";
   if (!std::filesystem::is_regular_file(data_version_filename)) {
      SPDLOG_TRACE(
         "Skipping {}. it does not contain the data version file {}, which "
         "confirms a finished and valid data source",
         path.string(),
         data_version_filename.string()
      );
      return std::nullopt;
   }
   auto data_version_in_file = silo::DataVersion::fromFile(data_version_filename);
   if (data_version_in_file == std::nullopt) {
      SPDLOG_TRACE(
         "Skipping {}. The data version in data_version.silo could not be parsed", path.string()
      );
      return std::nullopt;
   }
   if (data_version_in_file->getTimestamp() != folder_name_timestamp) {
      SPDLOG_WARN(
         "Skipping {}. The data version in data_version.silo is not equal to the directory name",
         path.string()
      );
      return std::nullopt;
   }
   return data_version_in_file;
}

std::optional<std::pair<std::filesystem::path, silo::DataVersion>> silo_api::
   DatabaseDirectoryWatcher::getMostRecentDataDirectory(const std::filesystem::path& path) {
   SPDLOG_TRACE("Scanning path {} for valid data", path.string());
   std::vector<std::pair<std::filesystem::path, silo::DataVersion>> all_found_data;
   for (const auto& directory_entry : std::filesystem::directory_iterator{path}) {
      SPDLOG_TRACE("Checking directory entry {}", directory_entry.path().string());
      auto data_version = checkValidDataSource(directory_entry.path());
      if (data_version.has_value()) {
         SPDLOG_TRACE(
            "Found candidate data source {} with data version {}",
            directory_entry.path().string(),
            data_version->toString()
         );
         all_found_data.emplace_back(directory_entry.path(), std::move(*data_version));
      }
   }
   if (all_found_data.empty()) {
      SPDLOG_TRACE("Scan of path {} returned no valid data", path.string());
      return std::nullopt;
   }
   std::sort(
      all_found_data.begin(),
      all_found_data.end(),
      [](const auto& element1, const auto& element2) { return element1.second > element2.second; }
   );
   for (auto& entry : all_found_data) {
      if (entry.second.isCompatibleVersion()) {
         SPDLOG_TRACE(
            "Selected newest data which is compatible ({}) in path {} for ingestion",
            entry.second.toString(),
            entry.first.string()
         );
         return entry;
      }
      SPDLOG_WARN(
         "The database output {} is incompatible with the current SILO version {}.",
         entry.second.toString(),
         silo::DataVersion::CURRENT_SILO_SERIALIZATION_VERSION.value
      );
   }
   return std::nullopt;
}

void silo_api::DatabaseDirectoryWatcher::checkDirectoryForData(Poco::Timer& /*timer*/) {
   auto most_recent_database_state = getMostRecentDataDirectory(path);

   if (most_recent_database_state == std::nullopt) {
      SPDLOG_INFO("No data found in {} for ingestion", path.string());
      return;
   }

   {
      try {
         const auto current_data_version_timestamp =
            database_mutex.getDatabase().database.getDataVersionTimestamp();
         const auto most_recent_data_version_timestamp_found =
            most_recent_database_state->second.getTimestamp();
         if (current_data_version_timestamp >= most_recent_data_version_timestamp_found) {
            SPDLOG_TRACE(
               "Do not update data version to {} in path {}. Its version is not newer than the "
               "current version",
               most_recent_database_state->second.toString(),
               most_recent_database_state->first.string()
            );
            return;
         }
      } catch (const silo_api::UninitializedDatabaseException& exception) {
         SPDLOG_DEBUG("No database loaded yet - loading initial database next.");
      }
   }

   SPDLOG_INFO("New data version detected: {}", most_recent_database_state->first.string());
   try {
      database_mutex.setDatabase(silo::Database::loadDatabaseState(most_recent_database_state->first
      ));
   } catch (const std::exception& ex) {
      SPDLOG_ERROR(ex.what());
   } catch (const std::string& ex) {
      SPDLOG_ERROR(ex);
   } catch (...) {
      SPDLOG_ERROR("Loading cancelled with uncatchable (...) exception");
      const auto exception = std::current_exception();
      if (exception) {
         const auto* message = abi::__cxa_current_exception_type()->name();
         SPDLOG_ERROR("current_exception: {}", message);
      }
   }
   SPDLOG_INFO(
      "New database with version {} successfully loaded.",
      most_recent_database_state->first.string()
   );
}