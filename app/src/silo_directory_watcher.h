#pragma once

#include <Poco/Timer.h>

#include <rhydb/common/silo_directory.h>

#include "active_database.h"

namespace silo_app {

class SiloDirectoryWatcher {
   rhydb::SiloDirectory silo_directory;
   std::shared_ptr<ActiveDatabase> database_handle;
   Poco::Timer timer;

  public:
   SiloDirectoryWatcher(
      rhydb::SiloDirectory silo_directory,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   void checkDirectoryForData(Poco::Timer& timer);
};

}  // namespace silo_app
