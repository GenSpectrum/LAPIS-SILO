#pragma once

#include <filesystem>
#include <optional>

#include "silo/config/preprocessing_config.h"

namespace silo::config {
class AbstractConfig;
}

namespace silo_api {

const std::string DATA_DIRECTORY_OPTION = "dataDirectory";
const std::string MAX_CONNECTIONS_OPTION = "maxQueuedHttpConnections";
const std::string PARALLEL_THREADS_OPTION = "threadsForHttpConnections";
const std::string PORT_OPTION = "port";
const std::string ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION = "estimatedStartupTimeInMinutes";

struct RuntimeConfig {
   std::filesystem::path data_directory = silo::config::DEFAULT_OUTPUT_DIRECTORY;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
      estimated_startup_end;

   void overwrite(const silo::config::AbstractConfig& config);
};

}  // namespace silo_api
