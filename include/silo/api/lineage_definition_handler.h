#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "active_database.h"
#include "rest_resource.h"

namespace silo::api {

class LineageDefinitionHandler : public RestResource {
  private:
   std::shared_ptr<Database> database;
   std::string column_name;

  public:
   explicit LineageDefinitionHandler(std::shared_ptr<Database> database, std::string column_name);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};

}  // namespace silo::api
