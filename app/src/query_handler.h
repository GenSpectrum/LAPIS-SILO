#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <rhydb/config/runtime_config.h>

#include "active_database.h"
#include "rest_resource.h"

namespace silo_app {
class QueryHandler : public RestResource {
  private:
   rhydb::config::QueryOptions query_options;
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   explicit QueryHandler(
      std::shared_ptr<ActiveDatabase> database_handle,
      rhydb::config::QueryOptions query_options
   );

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo_app
