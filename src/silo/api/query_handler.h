#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "silo/api/active_database.h"
#include "silo/api/rest_resource.h"
#include "silo/config/runtime_config.h"

namespace silo::api {
class QueryHandler : public RestResource {
  private:
   config::QueryOptions query_options;
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   explicit QueryHandler(
      std::shared_ptr<ActiveDatabase> database_handle,
      config::QueryOptions query_options
   );

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
