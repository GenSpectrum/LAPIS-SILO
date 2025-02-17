#pragma once

#include <atomic>
#include <filesystem>
#include <optional>

#include <crow.h>

#include "silo/api/active_database.h"
#include "silo/common/data_version.h"

namespace silo::api {

class DatabaseDirectoryWatcher {
   std::atomic<bool> running;
   std::atomic<bool> stopped;
   std::filesystem::path path;
   std::shared_ptr<ActiveDatabase> active_database;

  public:
   DatabaseDirectoryWatcher(
      std::filesystem::path path,
      std::shared_ptr<ActiveDatabase> active_database
   );

   void start();

   void stop();

   void checkDirectoryForData();

   static std::optional<silo::DataVersion> checkValidDataSource(const std::filesystem::path& path);

   static std::optional<std::pair<std::filesystem::path, silo::DataVersion>>
   getMostRecentDataDirectory(const std::filesystem::path& path);
};

}  // namespace silo::api
