#include "silo/api/request_id_handler.h"

#include <spdlog/spdlog.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using boost::uuids::random_generator;

namespace silo::api {

RequestIdHandler::RequestIdHandler(std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler)
    : wrapped_handler(std::move(wrapped_handler)) {}

void RequestIdHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto request_id = getRequestId(request);
   response.set(REQUEST_ID_HEADER, request_id);

   wrapped_handler->handleRequest(request, response);
}

std::string RequestIdHandler::getRequestId(Poco::Net::HTTPServerRequest& request) {
   if (request.has(REQUEST_ID_HEADER)) {
      return request.get(REQUEST_ID_HEADER);
   }

   random_generator generator;
   const auto request_id = generator();
   return boost::uuids::to_string(request_id);
}

}  // namespace silo::api