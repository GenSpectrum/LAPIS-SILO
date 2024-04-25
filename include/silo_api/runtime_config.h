#pragma once

#include <filesystem>
#include <optional>

#include <Poco/Util/AbstractConfiguration.h>

#include "silo/preprocessing/preprocessing_config.h"

namespace silo_api {

const std::string DATA_DIRECTORY_OPTION = "dataDirectory";
const std::string DATA_DIRECTORY_ENV_OPTION = "SILO_DATA_DIRECTORY";
const std::string MAX_CONNECTIONS_OPTION = "maxQueuedHttpConnections";
const std::string MAX_CONNECTIONS_ENV_OPTION = "SILO_MAX_QUEUED_HTTP_CONNECTIONS";
const std::string PARALLEL_THREADS_OPTION = "threadsForHttpConnections";
const std::string PARALLEL_THREADS_ENV_OPTION = "SILO_THREADS_FOR_HTTP_CONNECTIONS";
const std::string PORT_OPTION = "port";
const std::string PORT_ENV_OPTION = "SILO_PORT";

struct RuntimeConfig {
   std::filesystem::path data_directory = silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;

   void overwriteFromFile(const std::filesystem::path& config_path);
   void overwriteFromEnvironmentVariables();
   void overwriteFromCommandLineArguments(const Poco::Util::AbstractConfiguration& config);
};

}  // namespace silo_api
