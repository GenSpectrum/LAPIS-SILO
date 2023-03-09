#include "silo_api/error.h"
#include <nlohmann/json.hpp>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace silo_api {
void NotFoundHandler::handleRequest(
   Poco::Net::HTTPServerRequest& /*request*/,
   Poco::Net::HTTPServerResponse& response
) {
   response.setContentType("application/json");
   response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(ErrorResponse{"Not found", "Resource does not exist"});
}
}  // namespace silo_api