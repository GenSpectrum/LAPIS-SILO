#pragma once

#include <filesystem>
#include <optional>

#include "silo/config/preprocessing_config.h"
#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

const AbstractConfigSource::Option DATA_DIRECTORY_OPTION{{"dataDirectory"}};
const AbstractConfigSource::Option MAX_CONNECTIONS_OPTION{{"maxQueuedHttpConnections"}};
const AbstractConfigSource::Option PARALLEL_THREADS_OPTION{{"threadsForHttpConnections"}};
const AbstractConfigSource::Option PORT_OPTION{{"port"}};
const AbstractConfigSource::Option ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION{
   {"estimatedStartupTimeInMinutes"}
};

struct ApiOptions {
   std::filesystem::path data_directory = silo::config::DEFAULT_OUTPUT_DIRECTORY;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
      estimated_startup_end;
};

struct RuntimeConfig {
   ApiOptions api_options;

   void overwrite(const silo::config::AbstractConfigSource& config);
};

}  // namespace silo::config
