#include "health_handler.h"

#include <map>
#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

namespace silo_app {

void HealthHandler::get(
   Poco::Net::HTTPServerRequest& /*request*/,
   Poco::Net::HTTPServerResponse& response
) {
   response.setContentType("application/json");
   response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
   response.send() << R"({"status":"UP"})";
}
}  // namespace silo_app
