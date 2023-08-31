#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "silo_api/rest_resource.h"

namespace silo_api {
class DatabaseMutex;

class InfoHandler : public RestResource {
  private:
   DatabaseMutex& database;

  public:
   explicit InfoHandler(DatabaseMutex& database);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo_api
