#ifndef SILO_DATABASE_MUTEX_H
#define SILO_DATABASE_MUTEX_H

#include <mutex>
#include <shared_mutex>

#include "silo/database.h"

namespace silo_api {

class FixedDatabase {
   std::shared_lock<std::shared_mutex> lock;

  public:
   FixedDatabase(const silo::Database& database, std::shared_lock<std::shared_mutex>&& mutex);

   const silo::Database& database;
};

class DatabaseMutex {
   std::shared_mutex mutex;
   silo::Database database;

  public:
   DatabaseMutex() = default;

   void setDatabase(silo::Database&& new_database);

   virtual FixedDatabase getDatabase();
};
}  // namespace silo_api

#endif  // SILO_DATABASE_MUTEX_H
