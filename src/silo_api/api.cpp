#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <cxxabi.h>

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
#include <boost/algorithm/string/join.hpp>

#include "silo/config/config_repository.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/storage/reference_genomes.h"
#include "silo_api/database_directory_watcher.h"
#include "silo_api/database_mutex.h"
#include "silo_api/logging.h"
#include "silo_api/request_handler_factory.h"
#include "silo_api/runtime_config.h"

static const std::string ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION = "estimatedStartupTimeInMinutes";
static const std::string PREPROCESSING_CONFIG_OPTION = "preprocessingConfig";
static const std::string DATABASE_CONFIG_OPTION = "databaseConfig";
static const std::string API_OPTION = "api";
static const std::string PREPROCESSING_OPTION = "preprocessing";

silo::preprocessing::PreprocessingConfig preprocessingConfig(
   const Poco::Util::AbstractConfiguration& config
) {
   silo::preprocessing::OptionalPreprocessingConfig default_preprocessing_config;
   if (std::filesystem::exists("./default_preprocessing_config.yaml")) {
      default_preprocessing_config = silo::preprocessing::PreprocessingConfigReader().readConfig(
         "./default_preprocessing_config.yaml"
      );
   }

   silo::preprocessing::OptionalPreprocessingConfig user_preprocessing_config;
   if (config.hasProperty(PREPROCESSING_CONFIG_OPTION)) {
      user_preprocessing_config = silo::preprocessing::PreprocessingConfigReader().readConfig(
         config.getString(PREPROCESSING_CONFIG_OPTION)
      );
   } else if (std::filesystem::exists("./preprocessing_config.yaml")) {
      user_preprocessing_config =
         silo::preprocessing::PreprocessingConfigReader().readConfig("./preprocessing_config.yaml");
   }

   const auto preprocessing_config =
      user_preprocessing_config.mergeValuesFromOrDefault(default_preprocessing_config);
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
                           .fullName(silo_api::DATA_DIRECTORY_OPTION)
                           .shortName("d")
                           .description("path to the preprocessed data")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding(silo_api::DATA_DIRECTORY_OPTION));

      options.addOption(Poco::Util::Option()
                           .fullName(silo_api::PORT_OPTION)
                           .description("port to listen to requests")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding(silo_api::PORT_OPTION));

      options.addOption(Poco::Util::Option()
                           .fullName("maxQueuedHttpConnections")
                           .description("maximum number of http connections")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding("maxQueuedHttpConnections"));

      options.addOption(Poco::Util::Option()
                           .fullName("threadsForHttpConnections")
                           .description("number of threads for http connections")
                           .required(false)
                           .repeatable(false)
                           .argument("NUMBER")
                           .binding("threadsForHttpConnections"));

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
            ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION,
            "t",
            "Estimated time in minutes that the initial loading of the database takes. "
            "As long as no database is loaded yet, SILO will throw a 503 error. "
            "This option allows SILO to compute a Retry-After header for the 503 response. ",
            false
         )
            .required(false)
            .repeatable(false)
            .argument("MINUTES", true)
            .binding(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)
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
   silo_api::StartupConfig getStartupConfig() {
      const auto now = std::chrono::system_clock::now();
      const auto estimated_startup_time_in_minutes =
         config().hasProperty(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION)
            ? std::optional(
                 std::chrono::minutes(config().getInt(ESTIMATED_STARTUP_TIME_IN_MINUTES_OPTION))
              )
            : std::nullopt;

      return {.start_time = now, .estimated_startup_time = estimated_startup_time_in_minutes};
   }

   int handleApi() {
      SPDLOG_INFO("Starting SILO API");
      silo_api::RuntimeConfig runtime_config;
      if (std::filesystem::exists("./runtime_config.yaml")) {
         runtime_config.overwriteFromFile("./runtime_config.yaml");
      }
      runtime_config.overwriteFromEnvironmentVariables();
      runtime_config.overwriteFromCommandLineArguments(config());

      silo_api::DatabaseMutex database_mutex;

      const Poco::Net::ServerSocket server_socket(runtime_config.port);

      const silo_api::DatabaseDirectoryWatcher watcher(
         runtime_config.data_directory, database_mutex
      );

      auto* const poco_parameter = new Poco::Net::HTTPServerParams;

      SPDLOG_INFO("Using {} queued http connections", runtime_config.max_connections);
      poco_parameter->setMaxQueued(runtime_config.max_connections);

      SPDLOG_INFO("Using {} threads for http connections", runtime_config.parallel_threads);
      poco_parameter->setMaxThreads(runtime_config.parallel_threads);

      Poco::Net::HTTPServer server(
         new silo_api::SiloRequestHandlerFactory(database_mutex, getStartupConfig()),
         server_socket,
         poco_parameter
      );

      SPDLOG_INFO("Listening on port {}", runtime_config.port);

      server.start();
      waitForTerminationRequest();
      server.stop();

      return Application::EXIT_OK;
   }

   silo::Database runPreprocessor(
      const silo::preprocessing::PreprocessingConfig& preprocessing_config
   ) {
      auto database_config = databaseConfig(config());

      SPDLOG_INFO("preprocessing - reading reference genome");
      const auto reference_genomes =
         silo::ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

      auto preprocessor = silo::preprocessing::Preprocessor(
         preprocessing_config, database_config, reference_genomes
      );

      return preprocessor.preprocess();
   }

   int handlePreprocessing() {
      SPDLOG_INFO("Starting SILO preprocessing");
      try {
         const auto preprocessing_config = preprocessingConfig(config());

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
