#include "silo_api/rest_resource.h"

#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

#include "silo_api/error_request_handler.h"

namespace silo_api {

void methodNotAllowed(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   response.setContentType("application/json");
   response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
   response.send() << nlohmann::json(ErrorResponse{
      "Method not allowed", request.getMethod() + " is not allowed on resource " + request.getURI()
   });
}

void RestResource::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   if (request.getMethod() == "GET") {
      get(request, response);
      return;
   }
   if (request.getMethod() == "POST") {
      post(request, response);
      return;
   }
   methodNotAllowed(request, response);
}

[[maybe_unused]] void RestResource::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   methodNotAllowed(request, response);
}

void RestResource::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   methodNotAllowed(request, response);
}

}  // namespace silo_api