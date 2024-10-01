#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

void ApiOptions::overwrite(const silo::config::AbstractConfigSource& config) {
   if (auto value = config.getString(DATA_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG("Using dataDirectory passed via {}: {}", config.configType(), *value);
      data_directory = *value;
   }
   if (auto value = config.getInt32(MAX_CONNECTIONS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} passed via {}: {}",
         MAX_CONNECTIONS_OPTION.toString(),
         config.configType(),
         *value
      );
      max_connections = *value;
   }
   if (auto value = config.getInt32(PARALLEL_THREADS_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         PARALLEL_THREADS_OPTION.toString(),
         config.configType(),
         *value
      );
      parallel_threads = *value;
   }
   if (auto value = config.getUInt16(PORT_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} passed via {}: {}", PORT_OPTION.toString(), config.configType(), *value
      );
      port = *value;
   }
   if (auto value = config.getInt32(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} as passed via {}: {}",
         ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION.toString(),
         config.configType(),
         *value
      );
      const std::chrono::minutes minutes = std::chrono::minutes(*value);
      estimated_startup_end = std::chrono::system_clock::now() + minutes;
   }
}

void QueryOptions::overwrite(const silo::config::AbstractConfigSource& config) {
   if (auto value = config.getUInt64(MATERIALIZATION_CUTOFF_OPTION)) {
      SPDLOG_DEBUG(
         "Using {} passed via {}: {}",
         MATERIALIZATION_CUTOFF_OPTION.toString(),
         config.configType(),
         *value
      );
      materialization_cutoff = *value;
   }
}

void RuntimeConfig::overwrite(const silo::config::AbstractConfigSource& config) {
   api_options.overwrite(config);
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::RuntimeConfig>::format(
   const silo::config::RuntimeConfig& runtime_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{{{\n");
   const char* perhaps_comma = " ";

#define CODE_FOR_FIELD(TOPLEVEL_FIELD, FIELD_NAME) \
   fmt::format_to(                                 \
      ctx.out(),                                   \
      "{} {}: '{}'",                               \
      perhaps_comma,                               \
      #FIELD_NAME,                                 \
      runtime_config.TOPLEVEL_FIELD.FIELD_NAME     \
   );                                              \
   perhaps_comma = ",";

   // struct ApiOptions
   CODE_FOR_FIELD(api_options, data_directory);
   CODE_FOR_FIELD(api_options, max_connections);
   CODE_FOR_FIELD(api_options, parallel_threads);
   CODE_FOR_FIELD(api_options, port);
   CODE_FOR_FIELD(api_options, estimated_startup_end);

   fmt::format_to(ctx.out(), "}}, {{\n");
   perhaps_comma = " ";

   // struct QueryOptions
   CODE_FOR_FIELD(query_options, materialization_cutoff);

#undef CODE_FOR_FIELD

   return fmt::format_to(ctx.out(), "}}}}\n");
}
