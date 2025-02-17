#pragma once

#include <crow.h>

#include "silo/api/active_database.h"
#include "silo/api/rest_resource.h"

namespace silo::api {

class InfoHandler {
  public:
   static void get(
      std::shared_ptr<const Database> database,
      crow::request& request,
      crow::response& response
   );
};
}  // namespace silo::api
