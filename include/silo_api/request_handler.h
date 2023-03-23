#ifndef SILO_REQUEST_HANDLER_H
#define SILO_REQUEST_HANDLER_H

#include <memory>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

namespace silo_api {

struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class LoggingRequestHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;

  public:
   explicit LoggingRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};

class ErrorRequestHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;

  public:
   explicit ErrorRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};

class RestResource : public Poco::Net::HTTPRequestHandler {
   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;

   virtual void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

   virtual void post(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   );
};

}  // namespace silo_api

#endif  // SILO_REQUEST_HANDLER_H
