#pragma once

#include <shared_mutex>

#include "silo/database.h"

namespace silo_api {

class FixedDatabase {
   std::shared_lock<std::shared_mutex> lock;

  public:
   FixedDatabase(const silo::Database& database, std::shared_lock<std::shared_mutex>&& mutex);

   const silo::Database& database;
};

class UninitializedDatabaseException : public std::runtime_error {
  public:
   UninitializedDatabaseException()
       : std::runtime_error("Database not initialized yet") {}
};

class DatabaseMutex {
   std::shared_mutex mutex;
   silo::Database database;
   bool is_initialized = false;

  public:
   DatabaseMutex() = default;

   void setDatabase(silo::Database&& new_database);

   virtual FixedDatabase getDatabase();
};
}  // namespace silo_api
