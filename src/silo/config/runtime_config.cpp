#include "silo/config/runtime_config.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

const ConfigStruct API_OPTIONS_METADATA{
   "ApiOptions",
   {
      ConfigStructField{
         "dataDirectory",
         ConfigValue{
            .type_name = "path",
            .default_value = {DEFAULT_OUTPUT_DIRECTORY},
            .help_text =
               "The path to the directory with the data files (output from preprocessing).",
         },
      },
      ConfigStructField{
         "maxQueuedHttpConnections",
         ConfigValue{
            .type_name = "u32",  // XX change to this in C++?
            .default_value = {"64"},
            .help_text = "The maximum number of concurrent connections accepted at any time.",
         }
      },
      ConfigStructField{
         "threadsForHttpConnections",
         ConfigValue{
            .type_name = "u32",
            .default_value = {"4"},
            .help_text = "The number of worker threads.",
            // XX docs: how does this interact with MAX_CONNECTIONS_OPTION ?
         }
      },
      ConfigStructField{
         "port",
         ConfigValue{
            .type_name = "u16",
            .default_value = {"8081"},
            .help_text = "The port number on which to listen for incoming HTTP connections.",
         }
      },
      ConfigStructField{
         "estimatedStartupTimeInMinutes",
         ConfigValue{
            .type_name = "u32",
            // ^ XX and that is the type for an intermediary value
            //   here; but really separate that step?
            .default_value = {},
            .help_text =
               "Estimated time in minutes that the initial loading of the database takes. \n"
               "As long as no database is loaded yet, SILO will throw a 503 error. \n"
               "This option allows SILO to compute a Retry-After header for the 503 response."
         }
      },
   }
};

const ConfigStruct QUERY_OPTIONS_METADATA{
   "QueryOptions",
   {
      ConfigStructField{
         "materializationCutoff",
         ConfigValue{
            .type_name = "usize",
            .default_value = {"10000"},
            .help_text =
               "Above how many records in a result set the result rows are to be constructed\n"
               "lazily (by streaming).",
         }
      },
   }
};

const ConfigStruct RUNTIME_CONFIG_METADATA{
   "siloServer",
   {ConfigStructField{
       "help",
       ConfigValue{
          .type_name = "bool",
          .default_value = {},
          .help_text = "Show help text.",
       }
    },
    ConfigStructField{
       "runtimeConfig",
       ConfigValue{
          .type_name = "path",
          .default_value = {},
          .help_text = "Path to config file in YAML format.",
       }
    },
    ConfigStructField{
       "preprocessingConfig",
       ConfigValue{
          .type_name = "ignored",
          .default_value = {},
          .help_text = "Ignored so that defaults can be provided via env vars for both \n"
                       "execution modes of the multi-call binary.",
       }
    },

    ConfigStructField{
       "api",
       &API_OPTIONS_METADATA,
    },
    ConfigStructField{"query", &QUERY_OPTIONS_METADATA}}
};

void ApiOptions::overwriteFromParents(
   const ConsList<std::string>& parents,
   const VerifiedConfigSource& config_source
) {
   using ::config::config_source_interface::get;
   using ::config::config_source_interface::set;

   // XX why did I not give option<path> here for 2nd argument?
   set<std::filesystem::path, std::filesystem::path>(
      data_directory, config_source, parents, "dataDirectory"
   );
   set<int32_t, int32_t>(max_connections, config_source, parents, "maxQueuedHttpConnections");
   set<int32_t, int32_t>(parallel_threads, config_source, parents, "threadsForHttpConnections");
   set<uint16_t, uint16_t>(port, config_source, parents, "port");

   // But estimated_startup_end is a derived value:
   if (auto value = get<uint32_t>(config_source, parents, "estimatedStartupTimeInMinutes")) {
      const std::chrono::minutes minutes = std::chrono::minutes(*value);
      estimated_startup_end = std::chrono::system_clock::now() + minutes;
   }
}

void QueryOptions::overwriteFromParents(
   const ConsList<std::string>& parents,
   const VerifiedConfigSource& config_source
) {
   using ::config::config_source_interface::set;
   set<size_t, size_t>(materialization_cutoff, config_source, parents, "materializationCutoff");
}

bool RuntimeConfig::asksForHelp() const {
   return help;
}

std::optional<std::filesystem::path> RuntimeConfig::configPath() const {
   return runtime_config;
}

void RuntimeConfig::overwriteFromParents(
   const ConsList<std::string>& parents,
   const VerifiedConfigSource& config_source
) {
   using ::config::config_source_interface::set;

   set<bool, bool>(help, config_source, parents, "help");
   set<std::filesystem::path, decltype(runtime_config)>(
      runtime_config, config_source, parents, "runtimeConfig"
   );
   set<Ignored, decltype(preprocessing_config)>(
      preprocessing_config, config_source, parents, "preprocessingConfig"
   );

   api_options.overwriteFromParents(parents.cons("api"), config_source);
   query_options.overwriteFromParents(parents.cons("query"), config_source);
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::RuntimeConfig>::format(
   const silo::config::RuntimeConfig& runtime_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{{{\n");
   std::string_view perhaps_comma = " ";

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
