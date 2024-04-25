#pragma once

#include <filesystem>
#include <optional>

#include <Poco/Util/AbstractConfiguration.h>

#include "silo/preprocessing/preprocessing_config.h"

namespace silo_api {

static const std::string DATA_DIRECTORY_OPTION = "dataDirectory";
static const std::string MAX_CONNECTIONS_OPTION = "maxQueuedHttpConnections";
static const std::string PARALLEL_THREADS_OPTION = "threadsForHttpConnections";
static const std::string PORT_OPTION = "port";

struct RuntimeConfig {
   std::filesystem::path data_directory = silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;

   void overwriteFromFile(const std::filesystem::path& config_path);
   void overwriteFromCommandLineArguments(const Poco::Util::AbstractConfiguration& config);
};

}  // namespace silo_api
