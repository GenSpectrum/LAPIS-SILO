#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "config/source/yaml_file.h"
#include "silo/common/fmt_formatters.h"

namespace {
using silo::config::ConfigKeyPath;
using silo::config::YamlFile;

// Using functions instead of global variables because of
// initialization order issues.

ConfigKeyPath runtimeConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("runtimeConfig");
}
ConfigKeyPath defaultRuntimeConfigOptionKey() {
   return YamlFile::stringToConfigKeyPath("defaultRuntimeConfig");
}
ConfigKeyPath dataDirectoryOptionKey() {
   return YamlFile::stringToConfigKeyPath("dataDirectory");
}
ConfigKeyPath apiPortOptionKey() {
   return YamlFile::stringToConfigKeyPath("api: port");
}
ConfigKeyPath apiMaxConnectionsOptionKey() {
   return YamlFile::stringToConfigKeyPath("api: maxQueuedHttpConnections");
}
ConfigKeyPath apiParallelThreadsOptionKey() {
   return YamlFile::stringToConfigKeyPath("api: threadsForHttpConnections");
}
ConfigKeyPath apiEstimatedStartupTimeOptionKey() {
   return YamlFile::stringToConfigKeyPath("api: estimatedStartupTimeInMinutes");
}
ConfigKeyPath queryMaterializationOptionKey() {
   return YamlFile::stringToConfigKeyPath("query: materializationCutoff");
}

}  // namespace

namespace silo::config {

ConfigSpecification RuntimeConfig::getConfigSpecification() {
   return {
      .program_name = "silo api",
      .fields =
         {
            ConfigAttributeSpecification::createWithoutDefault(
               runtimeConfigOptionKey(),
               ConfigValueType::PATH,
               "Path to config file in YAML format."
            ),
            ConfigAttributeSpecification::createWithoutDefault(
               defaultRuntimeConfigOptionKey(),
               ConfigValueType::PATH,
               "Path to config file in YAML format with default values. "
               "This path will often be set by an environment variable, thus "
               "providing defaults to a silo in a specific environment (e.g. Docker)"
            ),
            ConfigAttributeSpecification::createWithDefault(
               dataDirectoryOptionKey(),
               ConfigValue::fromPath(DEFAULT_OUTPUT_DIRECTORY),
               "The path to the directory with the data files (output from preprocessing)."
            ),
            ConfigAttributeSpecification::createWithDefault(
               apiPortOptionKey(),
               ConfigValue::fromUint16(8081),
               "The port number on which to listen for incoming HTTP connections."
            ),
            ConfigAttributeSpecification::createWithDefault(
               apiMaxConnectionsOptionKey(),
               ConfigValue::fromInt32(64),
               "The maximum number of concurrent connections accepted at any time."
            ),
            ConfigAttributeSpecification::createWithDefault(
               apiParallelThreadsOptionKey(),
               ConfigValue::fromInt32(4),
               "The number of worker threads."
            ),
            ConfigAttributeSpecification::createWithoutDefault(
               apiEstimatedStartupTimeOptionKey(),
               ConfigValueType::UINT32,
               "Estimated time in minutes that the initial loading of the database takes. \n"
               "As long as no database is loaded yet, SILO will throw a 503 error. \n"
               "This option allows SILO to compute a Retry-After header for the 503 response."
            ),
            ConfigAttributeSpecification::createWithDefault(
               queryMaterializationOptionKey(),
               ConfigValue::fromUint32(10000),
               "Above how many records in a result set the result rows are to be constructed\n"
               "lazily (by streaming)."
            ),
         }
   };
}

RuntimeConfig::RuntimeConfig() {
   overwriteFrom(getConfigSpecification().getConfigSourceFromDefaults());
}

std::vector<std::filesystem::path> RuntimeConfig::getConfigPaths() const {
   std::vector<std::filesystem::path> result;
   if (default_runtime_config.has_value()) {
      result.emplace_back(default_runtime_config.value());
   }
   if (runtime_config.has_value()) {
      result.emplace_back(runtime_config.value());
   }
   return result;
}

void RuntimeConfig::overwriteFrom(const VerifiedConfigAttributes& config_source) {
   if (auto var = config_source.getPath(runtimeConfigOptionKey())) {
      runtime_config = var.value();
   }
   if (auto var = config_source.getPath(defaultRuntimeConfigOptionKey())) {
      default_runtime_config = var.value();
   }
   if (auto var = config_source.getPath(dataDirectoryOptionKey())) {
      data_directory = var.value();
   }
   if (auto var = config_source.getUint16(apiPortOptionKey())) {
      api_options.port = var.value();
   }
   if (auto var = config_source.getInt32(apiMaxConnectionsOptionKey())) {
      api_options.max_connections = var.value();
   }
   if (auto var = config_source.getInt32(apiParallelThreadsOptionKey())) {
      api_options.parallel_threads = var.value();
   }
   // But estimated_startup_end is a derived value:
   if (auto var = config_source.getUint32(apiEstimatedStartupTimeOptionKey())) {
      const std::chrono::minutes minutes = std::chrono::minutes(var.value());
      api_options.estimated_startup_end = std::chrono::system_clock::now() + minutes;
   }
   if (auto var = config_source.getUint32(queryMaterializationOptionKey())) {
      query_options.materialization_cutoff = var.value();
   }
}

}  // namespace silo::config

namespace silo::common {
// Using a macro to get the declared variable name #FIELD_NAME
#define ADD_FIELD_TO_RESULT(VARIABLE, FIELD_NAME) \
   result += fmt::format("{}: {}, ", #FIELD_NAME, toDebugString(VARIABLE.FIELD_NAME))

std::string toDebugString(const silo::config::ApiOptions& api_options) {
   std::string result = "{";
   ADD_FIELD_TO_RESULT(api_options, max_connections);
   ADD_FIELD_TO_RESULT(api_options, parallel_threads);
   ADD_FIELD_TO_RESULT(api_options, port);
   ADD_FIELD_TO_RESULT(api_options, estimated_startup_end);
   result += "}";
   return result;
}

std::string toDebugString(const silo::config::QueryOptions& query_options) {
   std::string result = "{";
   ADD_FIELD_TO_RESULT(query_options, materialization_cutoff);
   result += "}";
   return result;
}

std::string toDebugString(const silo::config::RuntimeConfig& runtime_config) {
   std::string result = "{";
   ADD_FIELD_TO_RESULT(runtime_config, data_directory);
   ADD_FIELD_TO_RESULT(runtime_config, api_options);
   ADD_FIELD_TO_RESULT(runtime_config, query_options);
   result += "}";
   return result;
}
#undef ADD_FIELD_TO_RESULT
}  // namespace silo::common

[[maybe_unused]] auto fmt::formatter<silo::config::RuntimeConfig>::format(
   const silo::config::RuntimeConfig& runtime_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(ctx.out(), "{}", silo::common::toDebugString(runtime_config));
}
