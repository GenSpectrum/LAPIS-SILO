#pragma once

#include <memory>

#include "silo/database.h"

namespace silo::api {

class UninitializedDatabaseException : public std::runtime_error {
  public:
   UninitializedDatabaseException()
       : std::runtime_error("Database not initialized yet") {}
};

class ActiveDatabase {
   std::shared_ptr<silo::Database> database;

  public:
   ActiveDatabase() = default;
   ActiveDatabase(const ActiveDatabase& other) = delete;
   ActiveDatabase(ActiveDatabase&& other) = delete;
   ActiveDatabase& operator=(const ActiveDatabase& other) = delete;
   ActiveDatabase& operator=(ActiveDatabase&& other) = delete;

   void setActiveDatabase(silo::Database&& new_database);

   std::shared_ptr<silo::Database> getActiveDatabase();
};

}  // namespace silo::api
