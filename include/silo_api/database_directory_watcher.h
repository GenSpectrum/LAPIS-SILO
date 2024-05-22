#pragma once

#include <filesystem>
#include <optional>

#include <Poco/Timer.h>

#include "silo/common/data_version.h"

namespace silo_api {
class DatabaseMutex;
}

namespace silo_api {

class DatabaseDirectoryWatcher {
   std::filesystem::path path;
   DatabaseMutex& database_mutex;
   Poco::Timer timer;

  public:
   DatabaseDirectoryWatcher(std::filesystem::path path, DatabaseMutex& database_mutex);

   void checkDirectoryForData(Poco::Timer& timer);

   static std::optional<silo::DataVersion> checkValidDataSource(const std::filesystem::path& path);

   static std::optional<std::pair<std::filesystem::path, silo::DataVersion>>
   getMostRecentDataDirectory(const std::filesystem::path& path);
};

}  // namespace silo_api
