#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <nlohmann/json.hpp>

namespace silo_api {

struct StartupConfig {
   std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_time;
   std::optional<std::chrono::minutes> estimated_startup_time;
};

struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class ErrorRequestHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;
   const StartupConfig& startup_config;

  public:
   explicit ErrorRequestHandler(
      Poco::Net::HTTPRequestHandler* wrapped_handler,
      const StartupConfig& startup_config
   );

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;

  private:
   std::optional<std::string> computeRetryAfterHintForStartupTime();
};

};  // namespace silo_api
