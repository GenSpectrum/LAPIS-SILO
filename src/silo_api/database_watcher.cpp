#include "silo_api/database_watcher.h"

#include <ranges>

#include <boost/algorithm/string/join.hpp>

namespace {

std::optional<silo::Database> findInitialDatabase(const std::filesystem::path& path) {
   SPDLOG_INFO("Scanning path {} for valid data", path.string());
   std::vector<std::pair<std::filesystem::path, silo::DataVersion>> all_found_data;
   for (const auto& directory_entry : std::filesystem::directory_iterator{path}) {
      SPDLOG_INFO("Checking directory entry {}", directory_entry.path().string());
      if (!directory_entry.is_directory()) {
         SPDLOG_INFO("Skipping {} because it is not a directory", directory_entry.path().string());
         continue;
      }
      auto data_version = silo::DataVersion::fromString(directory_entry.path().filename().string());
      if (data_version == std::nullopt) {
         SPDLOG_INFO(
            "Skipping {}. Its name is not a valid data version.", directory_entry.path().string()
         );
         continue;
      }
      SPDLOG_INFO(
         "Found candidate data source {} with data version {}",
         directory_entry.path().string(),
         data_version->toString()
      );
      all_found_data.emplace_back(directory_entry.path(), std::move(*data_version));
   }
   if (all_found_data.empty()) {
      SPDLOG_INFO("No previous data found, place data in {} for ingestion", path.string());
      return std::nullopt;
   }
   if (all_found_data.size() == 1) {
      const std::filesystem::path data_path = all_found_data.at(0).first;
      SPDLOG_INFO("Using previous data: {}", data_path.string());
      return silo::Database::loadDatabaseState(data_path);
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
   return silo::Database::loadDatabaseState(max_element->first);
}

}  // namespace

silo_api::DatabaseWatcher::DatabaseWatcher(const std::string& path, DatabaseMutex& database_mutex)
    : watcher(path),
      database_mutex(database_mutex) {
   watcher.itemAdded += Poco::delegate(this, &DatabaseWatcher::onItemAdded);
   auto initial_database = findInitialDatabase(path);
   if (initial_database.has_value()) {
      database_mutex.setDatabase(std::move(*initial_database));
   }
}

void silo_api::DatabaseWatcher::onItemAdded(const Poco::DirectoryWatcher::DirectoryEvent& event) {
   SPDLOG_TRACE(
      "Item {} was added to the folder {}", watcher.directory().path(), event.item.path()
   );
   if (!event.item.isDirectory()) {
      SPDLOG_INFO("Ignoring item added event: {}. It is not a folder", event.item.path());
      return;
   }

   const std::filesystem::path path = event.item.path();
   auto new_data_version = silo::DataVersion::fromString(path.filename().string());
   if (new_data_version == std::nullopt) {
      SPDLOG_INFO(
         "Ignoring item added event: {}. Its name {} is not a valid data version.",
         event.item.path(),
         path.filename().string()
      );
      return;
   }
   {
      const auto fixed_database = database_mutex.getDatabase();
      if (!(fixed_database.database.getDataVersion() < new_data_version.value())) {
         SPDLOG_INFO(
            "Ignoring item added event: {}. Its version is not newer than the current version",
            event.item.path()
         );
         return;
      }
   }

   SPDLOG_INFO("New data version detected: {}", event.item.path());
   const std::string new_version_path = event.item.path();
   auto database = silo::Database::loadDatabaseState(new_version_path);

   database_mutex.setDatabase(silo::Database::loadDatabaseState(new_version_path));
}