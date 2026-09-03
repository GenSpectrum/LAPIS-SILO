#pragma once

#include <Poco/Timer.h>

#include <rhydb/common/rhydb_directory.h>

#include "active_database.h"

namespace rhydb_app {

class RhyDBDirectoryWatcher {
   rhydb::RhyDBDirectory rhydb_directory;
   std::shared_ptr<ActiveDatabase> database_handle;
   Poco::Timer timer;

  public:
   RhyDBDirectoryWatcher(
      rhydb::RhyDBDirectory rhydb_directory,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   void checkDirectoryForData(Poco::Timer& timer);
};

}  // namespace rhydb_app
