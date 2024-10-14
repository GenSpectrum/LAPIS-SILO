#include "silo_api/database_mutex.h"

#include <atomic>
#include <utility>

#include "silo/database.h"

namespace silo_api {

void silo_api::DatabaseMutex::setDatabase(silo::Database&& new_database) {
   auto new_database_pointer = std::make_shared<silo::Database>(std::move(new_database));

   std::atomic_store(&database, new_database_pointer);
   is_initialized = true;
}

std::shared_ptr<silo::Database> silo_api::DatabaseMutex::getDatabase() {
   if (!is_initialized) {
      throw silo_api::UninitializedDatabaseException();
   }
   return std::atomic_load(&database);
}

}  // namespace silo_api
