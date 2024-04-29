#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <nlohmann/json.hpp>

#include "silo/config/runtime_config.h"

namespace silo_api {

struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class ErrorRequestHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;
   const RuntimeConfig& runtime_config;

  public:
   explicit ErrorRequestHandler(
      Poco::Net::HTTPRequestHandler* wrapped_handler,
      const RuntimeConfig& runtime_config
   );

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;

  private:
   std::optional<std::string> computeRetryAfterHintForStartupTime();
};

};  // namespace silo_api
