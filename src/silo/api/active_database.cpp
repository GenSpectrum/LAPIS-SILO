#include "silo/api/active_database.h"

#if defined(__linux__)
#include <malloc.h>
#endif

#include <atomic>
#include <utility>

#include <spdlog/spdlog.h>

#include "silo/common/data_version.h"
#include "silo/database.h"

namespace {
void monitorReferenceCountThenTrimAllocations(
   silo::DataVersion::Timestamp data_version,
   std::weak_ptr<silo::Database> weak_ptr
) {
   while (true) {
      if (auto shared = weak_ptr.lock()) {
         SPDLOG_INFO(
            "Some requests on the old database with version {} are still running. "
            "Current reference count: {}",
            data_version.value,
            shared.use_count()
         );
      } else {
         SPDLOG_INFO(
            "No more references to the old database with version {} present. Stopping monitoring.",
            data_version.value
         );
#if defined(__linux__)
         SPDLOG_INFO("Manually invoking malloc_trim() to give back memory to OS.");
         malloc_trim(0);
#endif
         break;
      }
      std::this_thread::sleep_for(std::chrono::seconds(5));
   }
}
}  // namespace

namespace silo::api {

void silo::api::ActiveDatabase::setActiveDatabase(silo::Database&& new_database) {
   auto active_database = std::atomic_load(&database);
   if (active_database != nullptr) {
      std::thread monitorThread(
         monitorReferenceCountThenTrimAllocations,
         database->getDataVersionTimestamp(),
         std::weak_ptr<silo::Database>(active_database)
      );
      monitorThread.detach();
   }

   auto new_database_pointer = std::make_shared<silo::Database>(std::move(new_database));

   std::atomic_store(&database, new_database_pointer);

   SPDLOG_INFO(
      "Swapped Database that is serving new incoming requests to new database with data version "
      "{}.",
      database->getDataVersionTimestamp().value
   );
}

std::shared_ptr<silo::Database> silo::api::ActiveDatabase::getActiveDatabase() {
   auto active_database = std::atomic_load(&database);
   if (active_database == nullptr) {
      throw silo::api::UninitializedDatabaseException();
   }
   return active_database;
}

}  // namespace silo::api
