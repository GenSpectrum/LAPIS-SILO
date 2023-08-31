#include "silo_api/not_found_handler.h"

#include <iosfwd>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

#include "silo_api/error_request_handler.h"

namespace silo_api {
void NotFoundHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   response.setContentType("application/json");
   response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(ErrorResponse{
      "Not found", "Resource " + request.getURI() + " does not exist"});
}
}  // namespace silo_api