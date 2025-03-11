#include "silo/api/silo_directory_watcher.h"

#include <cxxabi.h>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "silo/api/active_database.h"
#include "silo/common/data_version.h"
#include "silo/database.h"

silo::api::SiloDirectoryWatcher::SiloDirectoryWatcher(
   silo::SiloDirectory silo_directory,
   std::shared_ptr<ActiveDatabase> database_handle
)
    : silo_directory(std::move(silo_directory)),
      database_handle(database_handle),
      timer(0, 2000) {
   timer.start(
      Poco::TimerCallback<SiloDirectoryWatcher>(*this, &SiloDirectoryWatcher::checkDirectoryForData)
   );
}

void silo::api::SiloDirectoryWatcher::checkDirectoryForData(Poco::Timer& /*timer*/) {
   auto maybe_most_recent_database_state = silo_directory.getMostRecentDataDirectory();

   if (maybe_most_recent_database_state == std::nullopt) {
      SPDLOG_INFO("No data found in {} for ingestion", silo_directory);
      return;
   }
   auto most_recent_database_state = maybe_most_recent_database_state.value();

   {
      try {
         const auto current_data_version_timestamp =
            database_handle->getActiveDatabase()->getDataVersionTimestamp();
         const auto most_recent_data_version_timestamp_found =
            most_recent_database_state.data_version.getTimestamp();
         if (current_data_version_timestamp >= most_recent_data_version_timestamp_found) {
            SPDLOG_TRACE(
               "Do not update data version to {} in path {}. Its version is not newer than the "
               "current version",
               most_recent_database_state.data_version.toString(),
               most_recent_database_state.path.string()
            );
            return;
         }
      } catch (const silo::api::UninitializedDatabaseException& exception) {
         SPDLOG_DEBUG("No database loaded yet - loading initial database next.");
      }
   }

   SPDLOG_INFO("New data version detected: {}", most_recent_database_state.path.string());
   try {
      database_handle->setActiveDatabase(
         silo::Database::loadDatabaseState(most_recent_database_state)
      );
      SPDLOG_INFO(
         "New database with version {} successfully loaded.",
         most_recent_database_state.path.string()
      );
      return;
   } catch (const std::exception& ex) {
      SPDLOG_ERROR(ex.what());
   } catch (const std::string& ex) {
      SPDLOG_ERROR(ex);
   } catch (...) {
      SPDLOG_ERROR("Loading cancelled with uncatchable (...) exception");
      const auto exception = std::current_exception();
      if (exception) {
         const auto* message = abi::__cxa_current_exception_type()->name();
         SPDLOG_ERROR("current_exception: {}", message);
      }
   }
   SPDLOG_INFO(
      "Did not load new database with version {} successfully.",
      most_recent_database_state.data_version.toString()
   );
}