#pragma once

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "active_database.h"
#include "rest_resource.h"

namespace silo::api {

class HealthHandler : public RestResource {
  public:
   explicit HealthHandler() = default;

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo::api
