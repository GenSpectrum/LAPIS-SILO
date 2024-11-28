#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "config/source/yaml_file.h"
#include "silo/common/fmt_formatters.h"

using silo::common::toDebugString;

namespace {
using silo::config::ConfigKeyPath;
using silo::config::YamlConfig;

ConfigKeyPath helpOptionKey() {
   return YamlConfig::stringToConfigKeyPath("help");
}
ConfigKeyPath runtimeConfigOptionKey() {
   return YamlConfig::stringToConfigKeyPath("runtimeConfig");
}
ConfigKeyPath defaultRuntimeConfigOptionKey() {
   return YamlConfig::stringToConfigKeyPath("defaultRuntimeConfig");
}
ConfigKeyPath dataDirectoryOptionKey() {
   return YamlConfig::stringToConfigKeyPath("dataDirectory");
}
ConfigKeyPath apiPortOptionKey() {
   return YamlConfig::stringToConfigKeyPath("api.port");
}
ConfigKeyPath apiMaxConnectionsOptionKey() {
   return YamlConfig::stringToConfigKeyPath("api.maxQueuedHttpConnections");
}
ConfigKeyPath apiParallelThreadsOptionKey() {
   return YamlConfig::stringToConfigKeyPath("api.threadsForHttpConnections");
}
ConfigKeyPath apiEstimatedStartupTimeOptionKey() {
   return YamlConfig::stringToConfigKeyPath("api.estimatedStartupTimeInMinutes");
}
ConfigKeyPath queryMaterializationOptionKey() {
   return YamlConfig::stringToConfigKeyPath("query.materializationCutoff");
}

}  // namespace

namespace silo::config {

ConfigSpecification RuntimeConfig::getConfigSpecification() {
   return {
      .program_name = "silo api",
      .fields =
         {
            ConfigValueSpecification::createWithoutDefault(
               helpOptionKey(), ConfigValueType::BOOL, "Show help text."
            ),
            ConfigValueSpecification::createWithoutDefault(
               runtimeConfigOptionKey(),
               ConfigValueType::PATH,
               "Path to config file in YAML format."
            ),
            ConfigValueSpecification::createWithoutDefault(
               defaultRuntimeConfigOptionKey(),
               ConfigValueType::PATH,
               "Path to config file in YAML format with default values. "
               "This path will often be set by an environment variable, thus "
               "providing defaults to a silo in a specific environment (e.g. Docker)"
            ),
            ConfigValueSpecification::createWithDefault(
               dataDirectoryOptionKey(),
               ConfigValue::fromPath(DEFAULT_OUTPUT_DIRECTORY),
               "The path to the directory with the data files (output from preprocessing)."
            ),
            ConfigValueSpecification::createWithDefault(
               apiPortOptionKey(),
               ConfigValue::fromUint16(8081),
               "The port number on which to listen for incoming HTTP connections."
            ),
            ConfigValueSpecification::createWithDefault(
               apiMaxConnectionsOptionKey(),
               ConfigValue::fromInt32(64),
               "The maximum number of concurrent connections accepted at any time."
            ),
            ConfigValueSpecification::createWithDefault(
               apiParallelThreadsOptionKey(),
               ConfigValue::fromInt32(4),
               "The number of worker threads."
            ),
            ConfigValueSpecification::createWithoutDefault(
               apiEstimatedStartupTimeOptionKey(),
               ConfigValueType::UINT32,
               "Estimated time in minutes that the initial loading of the database takes. \n"
               "As long as no database is loaded yet, SILO will throw a 503 error. \n"
               "This option allows SILO to compute a Retry-After header for the 503 response."
            ),
            ConfigValueSpecification::createWithDefault(
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

bool RuntimeConfig::asksForHelp() const {
   return help.has_value() && help.value();
}

std::vector<std::filesystem::path> RuntimeConfig::getConfigPaths() const {
   std::vector<std::filesystem::path> result;
   if(default_runtime_config.has_value()){
      result.emplace_back(default_runtime_config.value());
   }
   if(runtime_config.has_value()){
      result.emplace_back(runtime_config.value());
   }
   return result;
}

void RuntimeConfig::overwriteFrom(const VerifiedConfigSource& config_source) {
   if (auto var = config_source.getBool(helpOptionKey())) {
      help = var.value();
   }
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

#define CODE_FOR_FIELD(VARIABLE, FIELD_NAME) \
   fmt::format_to(ctx.out(), "{}: {}, ", #FIELD_NAME, toDebugString(VARIABLE.FIELD_NAME))

[[maybe_unused]] auto fmt::formatter<silo::config::ApiOptions>::format(
   const silo::config::ApiOptions& api_options,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{");
   CODE_FOR_FIELD(api_options, max_connections);
   CODE_FOR_FIELD(api_options, parallel_threads);
   CODE_FOR_FIELD(api_options, port);
   CODE_FOR_FIELD(api_options, estimated_startup_end);
   return fmt::format_to(ctx.out(), "}}");
}

[[maybe_unused]] auto fmt::formatter<silo::config::QueryOptions>::format(
   const silo::config::QueryOptions& query_options,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{");
   CODE_FOR_FIELD(query_options, materialization_cutoff);
   return fmt::format_to(ctx.out(), "}}");
}

[[maybe_unused]] auto fmt::formatter<silo::config::RuntimeConfig>::format(
   const silo::config::RuntimeConfig& runtime_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{");
   CODE_FOR_FIELD(runtime_config, data_directory);
   CODE_FOR_FIELD(runtime_config, api_options);
   CODE_FOR_FIELD(runtime_config, query_options);
   return fmt::format_to(ctx.out(), "}}");
}

#undef CODE_FOR_FIELD