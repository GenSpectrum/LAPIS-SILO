#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include "silo/config/util/abstract_config.h"

namespace silo_api {

void RuntimeConfig::overwrite(const silo::config::AbstractConfig& config) {
   if (config.hasProperty(DATA_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG(
         "Using dataDirectory passed via {}: {}",
         config.configType(),
         config.getString(DATA_DIRECTORY_OPTION)
      );
      data_directory = config.getString(DATA_DIRECTORY_OPTION);
   }
   if (config.hasProperty(MAX_CONNECTIONS_OPTION)) {
      SPDLOG_DEBUG(
         "Using maximum queued http connections passed via {}: {}",
         config.configType(),
         config.getInt32(MAX_CONNECTIONS_OPTION)
      );
      max_connections = config.getInt32(MAX_CONNECTIONS_OPTION);
   }
   if (config.hasProperty(PARALLEL_THREADS_OPTION)) {
      SPDLOG_DEBUG(
         "Using parallel threads for accepting http connections as passed via {}: {}",
         config.configType(),
         config.getInt32(PARALLEL_THREADS_OPTION)
      );
      parallel_threads = config.getInt32(PARALLEL_THREADS_OPTION);
   }
   if (config.hasProperty(PORT_OPTION)) {
      SPDLOG_DEBUG(
         "Using port passed via {}: {}", config.configType(), config.getString(PORT_OPTION)
      );
      port = config.getUInt32(PORT_OPTION);
   }
   if (config.hasProperty(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)) {
      SPDLOG_DEBUG(
         "Using estimated startup time in minutes as passed via {}: {}",
         config.configType(),
         config.getString(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)
      );
      const std::chrono::minutes minutes =
         std::chrono::minutes(config.getInt32(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION));
      estimated_startup_end = std::chrono::system_clock::now() + minutes;
   }
}

}  // namespace silo_api
