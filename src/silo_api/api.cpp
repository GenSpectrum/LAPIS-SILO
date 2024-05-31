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
#include <spdlog/spdlog.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/config/runtime_config.h"
#include "silo/config/util/abstract_config_source.h"
#include "silo/config/util/config_repository.h"
#include "silo/config/util/yaml_file.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo_api/command_line_arguments.h"
#include "silo_api/database_directory_watcher.h"
#include "silo_api/database_mutex.h"
#include "silo_api/environment_variables.h"
#include "silo_api/logging.h"
#include "silo_api/request_handler_factory.h"

static const std::string PREPROCESSING_CONFIG_OPTION = "preprocessingConfig";
static const std::string RUNTIME_CONFIG_OPTION = "runtimeConfig";
static const std::string DATABASE_CONFIG_OPTION = "databaseConfig";
static const std::string API_OPTION = "api";
static const std::string PREPROCESSING_OPTION = "preprocessing";

using silo::config::YamlFile;
using silo_api::CommandLineArguments;
using silo_api::EnvironmentVariables;

silo::config::PreprocessingConfig preprocessingConfig(
   const Poco::Util::AbstractConfiguration& config
) {
   silo::config::PreprocessingConfig preprocessing_config;
   if (std::filesystem::exists("./default_preprocessing_config.yaml")) {
      preprocessing_config.overwrite(YamlFile("./default_preprocessing_config.yaml"));
   }

   if (config.hasProperty(PREPROCESSING_CONFIG_OPTION)) {
      preprocessing_config.overwrite(YamlFile(config.getString(PREPROCESSING_CONFIG_OPTION)));
   } else if (std::filesystem::exists("./preprocessing_config.yaml")) {
      preprocessing_config.overwrite(YamlFile("./preprocessing_config.yaml"));
   }

   preprocessing_config.overwrite(EnvironmentVariables());
   preprocessing_config.overwrite(CommandLineArguments(config));
   preprocessing_config.validate();

   SPDLOG_INFO("Resulting preprocessing config: {}", preprocessing_config);
   return preprocessing_config;
}

silo::config::DatabaseConfig databaseConfig(const Poco::Util::AbstractConfiguration& config) {
   if (config.hasProperty(DATABASE_CONFIG_OPTION)) {
      return silo::config::ConfigRepository().getValidatedConfig(
         config.getString(DATABASE_CONFIG_OPTION)
      );
   }
   SPDLOG_DEBUG("databaseConfig not found in config file. Using default value: databaseConfig.yaml"
   );
   return silo::config::ConfigRepository().getValidatedConfig("database_config.yaml");
}

class SiloServer : public Poco::Util::ServerApplication {
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

      options.addOption(Poco::Util::Option()
                           .fullName(PREPROCESSING_CONFIG_OPTION)
                           .description("path to the preprocessing config file")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding(PREPROCESSING_CONFIG_OPTION));

      options.addOption(Poco::Util::Option()
                           .fullName(DATABASE_CONFIG_OPTION)
                           .description("path to the database config file")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding(DATABASE_CONFIG_OPTION));

      options.addOption(Poco::Util::Option()
                           .fullName(silo::config::DATA_DIRECTORY_OPTION.toCamelCase())
                           .shortName("d")
                           .description("path to the preprocessed data")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding(silo::config::DATA_DIRECTORY_OPTION.toCamelCase()));

      options.addOption(Poco::Util::Option()
                           .fullName(silo::config::PORT_OPTION.toCamelCase())
                           .description("port to listen to requests")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding(silo::config::PORT_OPTION.toCamelCase()));

      options.addOption(Poco::Util::Option()
                           .fullName(silo::config::MAX_CONNECTIONS_OPTION.toCamelCase())
                           .description("maximum number of http connections")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding(silo::config::MAX_CONNECTIONS_OPTION.toCamelCase()));

      options.addOption(Poco::Util::Option()
                           .fullName(silo::config::PARALLEL_THREADS_OPTION.toCamelCase())
                           .description("number of threads for http connections")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding(silo::config::PARALLEL_THREADS_OPTION.toCamelCase()));

      options.addOption(Poco::Util::Option()
                           .fullName(API_OPTION)
                           .shortName("a")
                           .description("Execution mode: start the SILO web interface")
                           .required(false)
                           .repeatable(false)
                           .binding(API_OPTION)
                           .group("executionMode"));

      options.addOption(
         Poco::Util::Option()
            .fullName(PREPROCESSING_OPTION)
            .shortName("p")
            .description("Execution mode: trigger the preprocessing pipeline to generate a "
                         "partitioned dataset that can be read by the database")
            .required(false)
            .repeatable(false)
            .binding(PREPROCESSING_OPTION)
            .group("executionMode")
      );

      options.addOption(
         Poco::Util::Option(
            silo::config::ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION.toCamelCase(),
            "t",
            "Estimated time in minutes that the initial loading of the database takes. "
            "As long as no database is loaded yet, SILO will throw a 503 error. "
            "This option allows SILO to compute a Retry-After header for the 503 response. ",
            false
         )
            .required(false)
            .repeatable(false)
            .argument("MINUTES", true)
            .binding(silo::config::ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION.toCamelCase())
      );
   }

   int main(const std::vector<std::string>& args) override {
      if (!args.empty()) {
         std::cout << "Unknown arguments provided: " << boost::algorithm::join(args, ", ")
                   << "\n\n";
         displayHelp("", "");
         return Application::EXIT_USAGE;
      }

      if (config().hasProperty(API_OPTION)) {
         return handleApi();
      }

      if (config().hasProperty(PREPROCESSING_OPTION)) {
         return handlePreprocessing();
      }

      std::cout << "No execution mode specified.\n\n";
      displayHelp("", "");
      return Application::EXIT_USAGE;
   }

  private:
   int handleApi() {
      SPDLOG_INFO("Starting SILO API");
      silo::config::RuntimeConfig runtime_config;
      if (config().hasProperty(RUNTIME_CONFIG_OPTION)) {
         runtime_config.overwrite(YamlFile(config().getString(RUNTIME_CONFIG_OPTION)));
      } else if (std::filesystem::exists("./runtime_config.yaml")) {
         runtime_config.overwrite(YamlFile("./runtime_config.yaml"));
      }
      runtime_config.overwrite(EnvironmentVariables());
      runtime_config.overwrite(CommandLineArguments(config()));

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

   silo::Database runPreprocessor(const silo::config::PreprocessingConfig& preprocessing_config) {
      auto database_config = databaseConfig(config());

      SPDLOG_INFO("preprocessing - reading reference genome");
      const auto reference_genomes =
         silo::ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

      SPDLOG_INFO("preprocessing - reading pango lineage alias");
      const auto alias_lookup = silo::PangoLineageAliasLookup::readFromFile(
         preprocessing_config.getPangoLineageDefinitionFilename()
      );

      auto preprocessor = silo::preprocessing::Preprocessor(
         preprocessing_config, database_config, reference_genomes, alias_lookup
      );

      return preprocessor.preprocess();
   }

   int handlePreprocessing() {
      SPDLOG_INFO("Starting SILO preprocessing");
      const auto preprocessing_config = preprocessingConfig(config());

      auto database = runPreprocessor(preprocessing_config);

      database.saveDatabaseState(preprocessing_config.getOutputDirectory());

      return Application::EXIT_OK;
   }

   void displayHelp(
      const std::string& /*name*/,
      const std::string& /*value*/
   ) {
      Poco::Util::HelpFormatter help_formatter(options());
      help_formatter.setCommand(commandName());
      help_formatter.setUsage("OPTIONS");
      help_formatter.setHeader("SILO - Sequence Indexing engine for Large Order of genomic data");
      help_formatter.format(std::cout);
   }
};

int main(int argc, char** argv) {
   setupLogger();

   SPDLOG_INFO("Starting SILO");

   SiloServer app;
   const auto return_code = app.run(argc, argv);

   SPDLOG_INFO("Stopping SILO");
   spdlog::default_logger()->flush();

   return return_code;
}
