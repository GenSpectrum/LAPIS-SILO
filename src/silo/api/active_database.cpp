#include "silo/api/active_database.h"

#include <atomic>
#include <utility>

#include "silo/common/data_version.h"
#include "silo/database.h"

namespace silo::api {

void silo::api::ActiveDatabase::setActiveDatabase(silo::Database&& new_database) {
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
