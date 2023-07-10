#include "silo_api/error_request_handler.h"

#include <cxxabi.h>

#include <Poco/Net/HTTPServerResponse.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace silo_api {
ErrorRequestHandler::ErrorRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler)
    : wrapped_handler(wrapped_handler){};

void ErrorRequestHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   try {
      wrapped_handler->handleRequest(request, response);
   } catch (const std::exception& exception) {
      SPDLOG_ERROR("Caught exception: {}", exception.what());

      response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{"Internal server error", exception.what()});
   } catch (...) {
      const auto exception = std::current_exception();
      const auto* message = exception ? abi::__cxa_current_exception_type()->name() : "null";
      SPDLOG_ERROR("Caught something unexpected: {}", message);

      response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);

      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         "Internal server error", "Caught something: " + std::string(message)});
   }
}
}  // namespace silo_api
