#pragma once

#include <filesystem>
#include <optional>

#include <Poco/Timer.h>

#include "silo/api/request_handler_factory.h"
#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"

namespace silo::api {

class SiloDirectoryWatcher {
   SiloDirectory silo_directory;
   std::shared_ptr<ActiveDatabase> database_handle;
   Poco::Timer timer;

  public:
   SiloDirectoryWatcher(
      SiloDirectory silo_directory,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   void checkDirectoryForData(Poco::Timer& timer);
};

}  // namespace silo::api
