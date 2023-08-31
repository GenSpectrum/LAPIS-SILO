#include "silo_api/database_mutex.h"

#include <mutex>
#include <utility>

#include "silo/database.h"

silo_api::FixedDatabase::FixedDatabase(
   const silo::Database& database,
   std::shared_lock<std::shared_mutex>&& mutex
)
    : lock(std::move(mutex)),
      database(database) {}

void silo_api::DatabaseMutex::setDatabase(silo::Database&& new_database) {
   const std::unique_lock lock(mutex);
   database = std::move(new_database);
}

silo_api::FixedDatabase silo_api::DatabaseMutex::getDatabase() {
   std::shared_lock<std::shared_mutex> lock(mutex);
   return {database, std::move(lock)};
}
