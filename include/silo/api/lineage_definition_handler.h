#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "database_mutex.h"
#include "rest_resource.h"

namespace silo::api {

class LineageDefinitionHandler : public RestResource {
  private:
   DatabaseMutex& database;
   std::string column_name;

  public:
   explicit LineageDefinitionHandler(DatabaseMutex& database, std::string column_name);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};

}  // namespace silo::api
