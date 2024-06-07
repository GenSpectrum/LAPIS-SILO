#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

void RuntimeConfig::overwrite(const silo::config::AbstractConfigSource& config) {
   if (auto value = config.getString(DATA_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG("Using dataDirectory passed via {}: {}", config.configType(), *value);
      api_options.data_directory = *value;
   }
   if (auto value = config.getInt32(MAX_CONNECTIONS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} passed via {}: {}",
         MAX_CONNECTIONS_OPTION.toString(),
         config.configType(),
         *value
      );
      api_options.max_connections = *value;
   }
   if (auto value = config.getInt32(PARALLEL_THREADS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         PARALLEL_THREADS_OPTION.toString(),
         config.configType(),
         *value
      );
      api_options.parallel_threads = *value;
   }
   if (auto value = config.getUInt32(PORT_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} passed via {}: {}", PORT_OPTION.toString(), config.configType(), *value
      );
      api_options.port = *value;
   }
   if (auto value = config.getInt32(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION.toString(),
         config.configType(),
         *value
      );
      const std::chrono::minutes minutes = std::chrono::minutes(*value);
      api_options.estimated_startup_end = std::chrono::system_clock::now() + minutes;
   }
}

}  // namespace silo::config
