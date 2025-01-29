#pragma once

#include <Poco/Util/ServerApplication.h>

#include "silo/config/runtime_config.h"

namespace silo::api {

class Api : public Poco::Util::ServerApplication {
  public:
   int runApi(const silo::config::RuntimeConfig& runtime_config);
};

}  // namespace silo::api
