#pragma once

#include <memory>

#include "silo/database.h"

namespace silo_api {

class UninitializedDatabaseException : public std::runtime_error {
  public:
   UninitializedDatabaseException()
       : std::runtime_error("Database not initialized yet") {}
};

class DatabaseMutex {
   bool is_initialized = false;
   std::shared_ptr<silo::Database> database;

  public:
   DatabaseMutex() = default;
   DatabaseMutex(const DatabaseMutex& other) = delete;
   DatabaseMutex(DatabaseMutex&& other) = delete;
   DatabaseMutex& operator=(const DatabaseMutex& other) = delete;
   DatabaseMutex& operator=(DatabaseMutex&& other) = delete;

   void setDatabase(silo::Database&& new_database);

   virtual std::shared_ptr<silo::Database> getDatabase();
};
}  // namespace silo_api
