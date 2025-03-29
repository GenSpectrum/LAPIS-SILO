#pragma once

#include <crow.h>

class CrowSpdlogAdapter : public crow::ILogHandler {
  public:
   CrowSpdlogAdapter() {}
   void log(std::string message, crow::LogLevel level);
};
