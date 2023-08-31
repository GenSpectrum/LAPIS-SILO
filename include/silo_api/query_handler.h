#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "silo_api/rest_resource.h"

namespace silo_api {
class DatabaseMutex;
}

namespace silo_api {
class QueryHandler : public RestResource {
  private:
   silo_api::DatabaseMutex& database_mutex;

  public:
   explicit QueryHandler(silo_api::DatabaseMutex& database);

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo_api
