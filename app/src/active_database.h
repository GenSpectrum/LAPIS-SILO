#pragma once

#include <memory>

#include <rhydb/database.h>

namespace rhydb_app {

class UninitializedDatabaseException : public std::runtime_error {
  public:
   UninitializedDatabaseException()
       : std::runtime_error("Database not initialized yet") {}
};

class ActiveDatabase {
   std::shared_ptr<rhydb::Database> database;

  public:
   ActiveDatabase() = default;
   ActiveDatabase(const ActiveDatabase& other) = delete;
   ActiveDatabase(ActiveDatabase&& other) = delete;
   ActiveDatabase& operator=(const ActiveDatabase& other) = delete;
   ActiveDatabase& operator=(ActiveDatabase&& other) = delete;

   void setActiveDatabase(rhydb::Database&& new_database);

   std::shared_ptr<rhydb::Database> getActiveDatabase();
};

}  // namespace rhydb_app
