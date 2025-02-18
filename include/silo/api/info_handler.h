#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "active_database.h"
#include "rest_resource.h"

namespace silo::api {

class InfoHandler : public RestResource {
  private:
   std::shared_ptr<Database> database;

  public:
   explicit InfoHandler(std::shared_ptr<Database> database);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
