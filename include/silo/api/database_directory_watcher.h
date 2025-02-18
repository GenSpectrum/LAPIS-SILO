#pragma once

#include <filesystem>
#include <optional>

#include <Poco/Timer.h>

#include "silo/api/request_handler_factory.h"
#include "silo/common/data_version.h"

namespace silo::api {

class DatabaseDirectoryWatcher {
   std::filesystem::path path;
   std::shared_ptr<ActiveDatabase> database_handle;
   Poco::Timer timer;

  public:
   DatabaseDirectoryWatcher(
      std::filesystem::path path,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   void checkDirectoryForData(Poco::Timer& timer);

   static std::optional<silo::DataVersion> checkValidDataSource(const std::filesystem::path& path);

   static std::optional<std::pair<std::filesystem::path, silo::DataVersion>>
   getMostRecentDataDirectory(const std::filesystem::path& path);
};

}  // namespace silo::api
