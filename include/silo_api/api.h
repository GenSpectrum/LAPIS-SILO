#pragma once

#include <Poco/Util/ServerApplication.h>

#include "silo/config/runtime_config.h"

class SiloServer : public Poco::Util::ServerApplication {
  public:
   int runApi(const silo::config::RuntimeConfig& runtime_config);
};