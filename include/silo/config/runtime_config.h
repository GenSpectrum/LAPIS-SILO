#pragma once

#include <filesystem>
#include <optional>

#include "silo/config/preprocessing_config.h"
#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

const AbstractConfigSource::Option DATA_DIRECTORY_OPTION{{"api", "dataDirectory"}};
const AbstractConfigSource::Option MAX_CONNECTIONS_OPTION{{"api", "maxQueuedHttpConnections"}};
const AbstractConfigSource::Option PARALLEL_THREADS_OPTION{{"api", "threadsForHttpConnections"}};
const AbstractConfigSource::Option PORT_OPTION{{"api", "port"}};
const AbstractConfigSource::Option ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION{
   {"api", "estimatedStartupTimeInMinutes"}
};

struct ApiOptions {
   std::filesystem::path data_directory = silo::config::DEFAULT_OUTPUT_DIRECTORY;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
      estimated_startup_end;

   void overwrite(const silo::config::AbstractConfigSource& config);
};

const AbstractConfigSource::Option MATERIALIZATION_CUTOFF_OPTION{{"query", "materializationCutoff"}
};

struct QueryOptions {
   size_t materialization_cutoff = 10000;

   void overwrite(const silo::config::AbstractConfigSource& config);
};

struct RuntimeConfig {
   ApiOptions api_options;
   QueryOptions query_options;

   void overwrite(const silo::config::AbstractConfigSource& config);
   void validate() const {};
};

}  // namespace silo::config
