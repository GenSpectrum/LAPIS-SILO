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
static const std::string DATA_DIRECTORY_OPTION = "dataDirectory";
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

std::filesystem::path dataDirectory(
   const Poco::Util::AbstractConfiguration& config,
   const silo_api::RuntimeConfig& runtime_config
) {
   if (config.hasProperty(DATA_DIRECTORY_OPTION)) {
      SPDLOG_DEBUG(
         "Using dataDirectory passed via command line argument: {}",
         config.getString(DATA_DIRECTORY_OPTION)
      );
      return config.getString(DATA_DIRECTORY_OPTION);
   }
   if (runtime_config.data_directory.has_value()) {
      SPDLOG_DEBUG(
         "Using dataDirectory from runtime config file: {}",
         runtime_config.data_directory.value().string()
      );
      return runtime_config.data_directory.value();
   }

   SPDLOG_DEBUG(
      "dataDirectory not found in specified. Using default value: {}",
      silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory
   );
   return silo::preprocessing::DEFAULT_OUTPUT_DIRECTORY.directory;
}

class SiloServer : public Poco::Util::ServerApplication {
  protected:
   [[maybe_unused]] void defineOptions(Poco::Util::OptionSet& options) override {
      ServerApplication::defineOptions(options);

      options.addOption(
         Poco::Util::Option("help", "h", "display help information on command line arguments")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::displayHelp))
      );

      options.addOption(
         Poco::Util::Option(
            PREPROCESSING_CONFIG_OPTION, "pc", "path to the preprocessing config file"
         )
            .required(false)
            .repeatable(false)
            .argument("PATH")
            .binding(PREPROCESSING_CONFIG_OPTION)
      );

      options.addOption(
         Poco::Util::Option(DATABASE_CONFIG_OPTION, "dc", "path to the database config file")
            .required(false)
            .repeatable(false)
            .argument("PATH")
            .binding(DATABASE_CONFIG_OPTION)
      );

      options.addOption(
         Poco::Util::Option(DATA_DIRECTORY_OPTION, "d", "path to the preprocessed data")
            .required(false)
            .repeatable(false)
            .argument("PATH")
            .binding(DATA_DIRECTORY_OPTION)
      );

      options.addOption(
         Poco::Util::Option(API_OPTION, "a", "Execution mode: start the SILO web interface")
            .required(false)
            .repeatable(false)
            .binding(API_OPTION)
            .group("executionMode")
      );

      options.addOption(Poco::Util::Option(
                           PREPROCESSING_OPTION,
                           "p",
                           "Execution mode: trigger the preprocessing pipeline to generate a "
                           "partitioned dataset that can be read by the database"
      )
                           .required(false)
                           .repeatable(false)
                           .binding(PREPROCESSING_OPTION)
                           .group("executionMode"));

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

      std::cout << "No execution mode specified."
                << "\n\n";
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

      return {now, estimated_startup_time_in_minutes};
   }

   int handleApi() {
      SPDLOG_INFO("Starting SILO API");
      const int port = 8081;

      silo_api::RuntimeConfig runtime_config;
      if (std::filesystem::exists("./runtime_config.yaml")) {
         runtime_config = silo_api::RuntimeConfig::readFromFile("./runtime_config.yaml");
      }

      const auto data_directory = dataDirectory(config(), runtime_config);

      silo_api::DatabaseMutex database_mutex;

      const Poco::Net::ServerSocket server_socket(port);

      const silo_api::DatabaseDirectoryWatcher watcher(data_directory, database_mutex);

      Poco::Net::HTTPServer server(
         new silo_api::SiloRequestHandlerFactory(database_mutex, getStartupConfig()),
         server_socket,
         new Poco::Net::HTTPServerParams
      );

      SPDLOG_INFO("Listening on port {}", port);

      server.start();
      waitForTerminationRequest();
      server.stop();

      return Application::EXIT_OK;
   };

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
   };

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
