#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "active_database.h"
#include "rest_resource.h"

namespace silo::api {
class QueryHandler : public RestResource {
  private:
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   explicit QueryHandler(std::shared_ptr<ActiveDatabase> database_handle);

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
