#pragma once

#include "silo/api/active_database.h"

namespace silo::api {

class LineageDefinitionHandler {
   std::shared_ptr<ActiveDatabase> active_database;

  public:
   static void get(
      std::shared_ptr<const Database> database,
      crow::request& request,
      crow::response& response,
      const std::string& column_name
   );
};

}  // namespace silo::api
