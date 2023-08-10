#ifndef SILO_DATABASE_WATCHER_H
#define SILO_DATABASE_WATCHER_H

#include <string>

#include <Poco/Delegate.h>
#include <Poco/DirectoryWatcher.h>
#include <Poco/Path.h>

#include "silo_api/database_mutex.h"

namespace silo_api {

class DatabaseWatcher {
   Poco::DirectoryWatcher watcher;
   DatabaseMutex& database_mutex;

  public:
   DatabaseWatcher(const std::string& path, DatabaseMutex& database_mutex);

   void onItemAdded(const Poco::DirectoryWatcher::DirectoryEvent& ev);
};
}  // namespace silo_api

#endif  // SILO_DATABASE_WATCHER_H
