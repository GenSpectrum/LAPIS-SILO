#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "database_mutex.h"
#include "rest_resource.h"

namespace silo::api {

class InfoHandler : public RestResource {
  private:
   DatabaseMutex& database;

  public:
   explicit InfoHandler(DatabaseMutex& database);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
