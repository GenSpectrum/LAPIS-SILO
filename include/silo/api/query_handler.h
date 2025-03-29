#pragma once

#include "silo/api/active_database.h"
#include "silo/api/rest_resource.h"

namespace silo::api {
class QueryHandler {
  public:
   static void post(
      std::shared_ptr<const Database> database,
      crow::request& request,
      crow::response& response
   );
};
}  // namespace silo::api
