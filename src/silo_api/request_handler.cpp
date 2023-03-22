#include "silo_api/request_handler.h"
#include <nlohmann/json.hpp>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <spdlog/spdlog.h>

namespace silo_api {

LoggingRequestHandler::LoggingRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler)
    : wrapped_handler(wrapped_handler){};

void LoggingRequestHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   SPDLOG_INFO("Handling {} {}", request.getMethod(), request.getURI());

   wrapped_handler->handleRequest(request, response);

   SPDLOG_INFO("Responding with status code {}", response.getStatus());
}

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
      const auto* message = exception ? exception.__cxa_exception_type()->name() : "null";
      SPDLOG_ERROR("Caught something unexpected: {}", message);

      response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);

      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         "Internal server error", "Caught something: " + std::string(message)});
   }
}

}  // namespace silo_api
