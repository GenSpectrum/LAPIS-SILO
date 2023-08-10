#include "silo_api/database_watcher.h"

namespace {

std::optional<silo::Database> findInitialDatabase(const std::filesystem::path& path) {
   SPDLOG_INFO("Scanning path {} for valid data", path.string());
   std::vector<std::pair<std::filesystem::path, silo::DataVersion>> all_found_data;
   for (auto& directory_entry : std::filesystem::directory_iterator{path}) {
      SPDLOG_INFO("Checking directory entry {}", directory_entry.path().string());
      if (!directory_entry.is_directory()) {
         SPDLOG_INFO("Skipping {} because it is not a directory", directory_entry.path().string());
         continue;
      }
      auto data_version = silo::DataVersion::fromString(directory_entry.path().filename().string());
      if (data_version == std::nullopt) {
         SPDLOG_INFO(
            "Skipping {}. Its name is not a valid data version.", path.filename().string()
         );
         continue;
      }
      SPDLOG_INFO(
         "Found candidate data source {} with data version {}",
         directory_entry.path().string(),
         data_version->toString()
      );
      all_found_data.emplace_back(directory_entry, *data_version);
   }
   if (all_found_data.empty()) {
      SPDLOG_INFO("No previous data found, place data in {} for ingestion", path.string());
      return std::nullopt;
   }
   if (all_found_data.size() == 1) {
      std::filesystem::path data_path = all_found_data.at(0).first;
      SPDLOG_INFO("Using previous data: {}", data_path.string());
      return silo::Database::loadDatabaseState(data_path);
   }
   auto max_element = std::max(
      all_found_data.begin(),
      all_found_data.end(),
      [](const auto& element1, const auto& element2) { return element1->second < element2->second; }
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

void silo_api::DatabaseWatcher::onItemAdded(const Poco::DirectoryWatcher::DirectoryEvent& ev) {
   SPDLOG_TRACE("Item {} was added to the folder {}", watcher.directory().path(), ev.item.path());
   if (!ev.item.isDirectory()) {
      SPDLOG_INFO("Ignoring item added event: {}. It is not a folder", ev.item.path());
      return;
   }

   std::filesystem::path path = ev.item.path();
   auto new_data_version = silo::DataVersion::fromString(path.filename().string());
   if (new_data_version == std::nullopt) {
      SPDLOG_INFO(
         "Ignoring item added event: {}. Its name {} is not a valid data version.",
         ev.item.path(),
         path.filename().string()
      );
      return;
   }
   {
      auto fixed_database = database_mutex.getDatabase();
      if (!(fixed_database.database.getDataVersion() < new_data_version.value())) {
         SPDLOG_INFO(
            "Ignoring item added event: {}. Its version is not newer than the current version",
            ev.item.path()
         );
         return;
      }
   }

   SPDLOG_INFO("New data version detected: {}", ev.item.path());
   const std::string newVersionPath = ev.item.path();
   auto database = silo::Database::loadDatabaseState(newVersionPath);

   database_mutex.setDatabase(silo::Database::loadDatabaseState(newVersionPath));
}