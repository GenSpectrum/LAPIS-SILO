#pragma once

#include "silo/config/runtime_config.h"

namespace silo::api {

class Api {
  public:
   int runApi(const silo::config::RuntimeConfig& runtime_config);
};

}  // namespace silo::api
