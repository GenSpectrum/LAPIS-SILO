#ifndef SILO_DATABASE_DIRECTORY_WATCHER_H
#define SILO_DATABASE_DIRECTORY_WATCHER_H

#include <string>

#include <Poco/Delegate.h>
#include <Poco/Path.h>
#include <Poco/Timer.h>

#include "silo_api/database_mutex.h"

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

#endif  // SILO_DATABASE_DIRECTORY_WATCHER_H
