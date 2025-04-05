#include "silo/api/crow_spdlog_adapter.h"

#include <spdlog/spdlog.h>

void CrowSpdlogAdapter::log(std::string message, crow::LogLevel level) {
   switch (level) {
      case crow::LogLevel::DEBUG:
         SPDLOG_DEBUG("crow: " + message);
         break;
      case crow::LogLevel::INFO:
         SPDLOG_INFO("crow: " + message);
         break;
      case crow::LogLevel::WARNING:
         SPDLOG_WARN("crow: " + message);
         break;
      case crow::LogLevel::ERROR:
         SPDLOG_WARN("crow: " + message);
         break;
      case crow::LogLevel::CRITICAL:
         SPDLOG_CRITICAL("crow: " + message);
         break;
   }
}
