#pragma once

#include <filesystem>

#include <Poco/Timer.h>

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
};
}  // namespace silo_api
