#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "database_mutex.h"
#include "rest_resource.h"

namespace silo::api {
class QueryHandler : public RestResource {
  private:
   silo::api::DatabaseMutex& database_mutex;

  public:
   explicit QueryHandler(silo::api::DatabaseMutex& database);

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
