#pragma once

#include <Poco/Timer.h>

#include <silo/common/silo_directory.h>

#include "active_database.h"

namespace silo_app {

class SiloDirectoryWatcher {
   silo::SiloDirectory silo_directory;
   std::shared_ptr<ActiveDatabase> database_handle;
   Poco::Timer timer;

  public:
   SiloDirectoryWatcher(
      silo::SiloDirectory silo_directory,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   void checkDirectoryForData(Poco::Timer& timer);
};

}  // namespace silo_app
