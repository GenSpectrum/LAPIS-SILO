#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <cxxabi.h>

#include <Poco/Environment.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

#include "silo/common/lineage_tree.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/config/runtime_config.h"
#include "silo/config/util/abstract_config_source.h"
#include "silo/config/util/config_repository.h"
#include "silo/config/util/yaml_file.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo_api/command_line_arguments.h"
#include "silo_api/database_directory_watcher.h"
#include "silo_api/database_mutex.h"
#include "silo_api/environment_variables.h"
#include "silo_api/logging.h"
#include "silo_api/request_handler_factory.h"

namespace {

// The first level of option processing purely accesses command
// line arguments (environment variables are ignored); thus we're
// using the kebab case style for command line arguments directly,
// without going through `AbstractConfigSource::Option`:

// For the siloPreprocessor command:
const std::string PREPROCESSING_CONFIG_OPTION = "preprocessing-config";
// For the siloServer command:
const std::string RUNTIME_CONFIG_OPTION = "runtime-config";

// The 3 config source principles
using silo::config::YamlFile;
using silo_api::CommandLineArguments;
using silo_api::EnvironmentVariables;

template <typename Config>
Config getConfig(
   const Poco::Util::AbstractConfiguration& cmdline_args,
   const std::string& commandline_option_name
) {
   Config config;
   if (cmdline_args.hasProperty(commandline_option_name)) {
      config.overwrite(YamlFile(cmdline_args.getString(commandline_option_name)));
   }

   config.overwrite(EnvironmentVariables());
   config.overwrite(CommandLineArguments(cmdline_args));
   config.validate();

   SPDLOG_INFO("Resulting config from {}: {}", commandline_option_name, config);
   return config;
}

Poco::Util::Option optionalNonRepeatableOption(
   const silo::config::AbstractConfigSource::Option& option,
   const std::string& description,
   const std::string& argumentType
) {
   std::string option_string = CommandLineArguments::asUnixOptionString(option);
   return Poco::Util::Option()
      .fullName(option_string)
      .description(description)
      .required(false)
      .repeatable(false)
      .argument(argumentType)
      // the key under which it will be retrieved, same as documented string:
      .binding(option_string);
}

enum class ExecutionMode { PREPROCESSING, API };

ExecutionMode executionModeFromPath(const std::filesystem::path& program_path) {
   const auto filename = program_path.filename();
   if (filename == "siloPreprocessor") {
      return ExecutionMode::PREPROCESSING;
   }
   if (filename == "siloServer") {
      return ExecutionMode::API;
   }
   throw std::runtime_error(fmt::format(
      "run as invalid filename '{}' (must be 'siloPreprocessing' or 'siloServer')",
      filename.string()
   ));
}

class SiloApp : public Poco::Util::ServerApplication {
  protected:
   void displayHelp(
      const std::string& /*name*/,
      const std::string& /*value*/
   ) {
      Poco::Util::HelpFormatter help_formatter(options());
      help_formatter.setCommand(commandName());
      help_formatter.setUsage("OPTIONS");
      help_formatter.setHeader("SILO - Sequence Indexing engine for Large Order of genomic data");
      help_formatter.format(std::cout);
      exit(0);
   }
};

class SiloPreprocessor : public SiloApp {
  protected:
   [[maybe_unused]] void defineOptions(Poco::Util::OptionSet& options) override {
      ServerApplication::defineOptions(options);

      options.addOption(Poco::Util::Option()
                           .fullName("help")
                           .shortName("h")
                           .description("display help information on command line arguments")
                           .required(false)
                           .repeatable(false)
                           .callback(Poco::Util::OptionCallback<SiloPreprocessor>(
                              this, &SiloPreprocessor::displayHelp
                           )));

      options.addOption(Poco::Util::Option()
                           .fullName(PREPROCESSING_CONFIG_OPTION)
                           .description("path to the preprocessing config file")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding(PREPROCESSING_CONFIG_OPTION));

      options.addOption(
         optionalNonRepeatableOption(
            silo::config::DATA_DIRECTORY_OPTION, "path to the preprocessed data", "PATH"
         )
            .shortName("d")
      );

      silo::config::PreprocessingConfig::addOptions(options);
   }

   static silo::Database runPreprocessor(
      const silo::config::PreprocessingConfig& preprocessing_config
   ) {
      auto database_config = silo::config::ConfigRepository().getValidatedConfig(
         preprocessing_config.getDatabaseConfigFilename()
      );

      SPDLOG_INFO("preprocessing - reading reference genome");
      const auto reference_genomes =
         silo::ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

      silo::common::LineageTreeAndIdMap lineage_definitions;
      if (auto lineage_file_name = preprocessing_config.getLineageDefinitionsFilename()) {
         SPDLOG_INFO(
            "preprocessing - read and verify the lineage tree '{}'",
            lineage_file_name.value().string()
         );
         lineage_definitions = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
            lineage_file_name.value()
         );
      }

      auto preprocessor = silo::preprocessing::Preprocessor(
         preprocessing_config, database_config, reference_genomes, std::move(lineage_definitions)
      );

      return preprocessor.preprocess();
   }

   int main(const std::vector<std::string>& /*args*/) override {
      SPDLOG_INFO("Starting SILO preprocessing");
      try {
         const auto preprocessing_config =
            getConfig<silo::config::PreprocessingConfig>(config(), PREPROCESSING_CONFIG_OPTION);

         auto database = runPreprocessor(preprocessing_config);

         database.saveDatabaseState(preprocessing_config.getOutputDirectory());
      } catch (const std::exception& ex) {
         SPDLOG_ERROR(ex.what());
         throw ex;
      } catch (const std::string& ex) {
         SPDLOG_ERROR(ex);
         return 1;
      } catch (...) {
         SPDLOG_ERROR("Preprocessing cancelled with uncatchable (...) exception");
         const auto exception = std::current_exception();
         if (exception) {
            const auto* message = abi::__cxa_current_exception_type()->name();
            SPDLOG_ERROR("current_exception: {}", message);
         }
         return 1;
      }
      return Application::EXIT_OK;
   }
};

class SiloServer : public SiloApp {
  protected:
   [[maybe_unused]] void defineOptions(Poco::Util::OptionSet& options) override {
      ServerApplication::defineOptions(options);

      options.addOption(
         Poco::Util::Option()
            .fullName("help")
            .shortName("h")
            .description("display help information on command line arguments")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::displayHelp))
      );

      options.addOption(optionalNonRepeatableOption(
         silo::config::PORT_OPTION, "port to listen to requests", "NUMBER"
      ));

      options.addOption(optionalNonRepeatableOption(
         silo::config::MAX_CONNECTIONS_OPTION, "maximum number of http connections", "NUMBER"
      ));

      options.addOption(optionalNonRepeatableOption(
         silo::config::PARALLEL_THREADS_OPTION, "number of threads for http connections", "NUMBER"
      ));

      options.addOption(optionalNonRepeatableOption(
         silo::config::ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION,
         "Estimated time in minutes that the initial loading of the database takes. "
         "As long as no database is loaded yet, SILO will throw a 503 error. "
         "This option allows SILO to compute a Retry-After header for the 503 response. ",
         "MINUTES"
      ));
   }

   int main(const std::vector<std::string>& args) override {
      if (!args.empty()) {
         std::cout << "Unknown arguments provided: " << boost::algorithm::join(args, ", ")
                   << "\n\n";
         displayHelp("", "");
         return Application::EXIT_USAGE;
      }

      const auto runtime_config =
         getConfig<silo::config::RuntimeConfig>(config(), RUNTIME_CONFIG_OPTION);

      SPDLOG_INFO("Starting SILO API");

      silo_api::DatabaseMutex database_mutex;

      const Poco::Net::ServerSocket server_socket(runtime_config.api_options.port);

      const silo_api::DatabaseDirectoryWatcher watcher(
         runtime_config.api_options.data_directory, database_mutex
      );

      auto* const poco_parameter = new Poco::Net::HTTPServerParams;

      SPDLOG_INFO("Using {} queued http connections", runtime_config.api_options.max_connections);
      poco_parameter->setMaxQueued(runtime_config.api_options.max_connections);

      SPDLOG_INFO(
         "Using {} threads for http connections", runtime_config.api_options.parallel_threads
      );
      poco_parameter->setMaxThreads(runtime_config.api_options.parallel_threads);

      Poco::Net::HTTPServer server(
         new silo_api::SiloRequestHandlerFactory(database_mutex, runtime_config),
         server_socket,
         poco_parameter
      );

      SPDLOG_INFO("Listening on port {}", runtime_config.api_options.port);

      server.start();
      waitForTerminationRequest();
      server.stop();

      return Application::EXIT_OK;
   }
};

}  // namespace

int run(int argc, char** argv) {
   SPDLOG_INFO("Starting SILO");

   int return_code;
   switch (executionModeFromPath(argv[0])) {
      case ExecutionMode::PREPROCESSING: {
         SiloPreprocessor app{};
         return_code = app.run(argc, argv);
         break;
      }
      case ExecutionMode::API: {
         SiloServer app{};
         return_code = app.run(argc, argv);
         break;
      }
   }

   SPDLOG_INFO("Stopping SILO");

   spdlog::default_logger()->flush();

   return return_code;
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char** argv) {
   setupLogger();

   if (getenv("DEBUG_EXCEPTION")) {
      return run(argc, argv);
   }
   int return_code;
   try {
      return_code = run(argc, argv);
   } catch (std::exception& ex) {
      SPDLOG_ERROR("{}", ex.what());
      return 1;
   };
   return return_code;
}
