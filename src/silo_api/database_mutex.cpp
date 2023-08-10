#include "silo_api/database_mutex.h"

silo_api::FixedDatabase::FixedDatabase(silo::Database& database, std::shared_mutex& mutex)
    : lock(mutex),
      database(database) {}

silo_api::DatabaseMutex::DatabaseMutex(silo::Database&& database)
    : database(std::move(database)) {}

void silo_api::DatabaseMutex::setDatabase(silo::Database&& db) {
   const std::unique_lock lock(mutex);
   database = std::move(db);
}

const silo_api::FixedDatabase silo_api::DatabaseMutex::getDatabase() {
   return FixedDatabase(database, mutex);
}
