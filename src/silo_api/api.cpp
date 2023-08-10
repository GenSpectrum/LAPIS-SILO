#include <filesystem>
#include <iostream>
#include <optional>
#include <shared_mutex>
#include <string>

#include <Poco/Delegate.h>
#include <Poco/DirectoryWatcher.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Path.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/config/config_repository.h"
#include "silo/database.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/query_engine/query_engine.h"
#include "silo_api/database_mutex.h"
#include "silo_api/database_watcher.h"
#include "silo_api/info_handler.h"
#include "silo_api/logging.h"
#include "silo_api/request_handler_factory.h"
#include "silo_api/runtime_config.h"

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
   if (config.hasProperty("preprocessingConfig")) {
      user_preprocessing_config = silo::preprocessing::PreprocessingConfigReader().readConfig(
         config.getString("preprocessingConfig")
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
   if (config.hasProperty("databaseConfig")) {
      return silo::config::ConfigRepository().getValidatedConfig(config.getString("databaseConfig")
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
   if (config.hasProperty("dataDirectory")) {
      SPDLOG_DEBUG(
         "Using dataDirectory passed via command line argument: {}",
         config.getString("dataDirectory")
      );
      return config.getString("dataDirectory");
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
         Poco::Util::Option("preprocessingConfig", "pc", "path to the preprocessing config file")
            .required(false)
            .repeatable(false)
            .argument("PATH")
            .binding("preprocessingConfig")
      );

      options.addOption(
         Poco::Util::Option("databaseConfig", "dc", "path to the database config file")
            .required(false)
            .repeatable(false)
            .argument("PATH")
            .binding("databaseConfig")
      );

      options.addOption(Poco::Util::Option("dataDirectory", "d", "path to the preprocessed data")
                           .required(false)
                           .repeatable(false)
                           .argument("PATH")
                           .binding("dataDirectory"));

      options.addOption(
         Poco::Util::Option("api", "a", "Execution mode: start the SILO web interface")
            .required(false)
            .repeatable(false)
            .binding("api")
            .group("executionMode")
      );

      options.addOption(Poco::Util::Option(
                           "preprocessing",
                           "p",
                           "Execution mode: trigger the preprocessing pipeline to generate a "
                           "partitioned dataset that can be read by the database"
      )
                           .required(false)
                           .repeatable(false)
                           .binding("preprocessing")
                           .group("executionMode"));
   }

   int main(const std::vector<std::string>& args) override {
      if (!args.empty()) {
         std::cout << "Unknown arguments provided: " << boost::algorithm::join(args, ", ")
                   << "\n\n";
         displayHelp("", "");
         return Application::EXIT_USAGE;
      }

      if (config().hasProperty("api")) {
         return handleApi();
      }

      if (config().hasProperty("preprocessing")) {
         return handlePreprocessing();
      }

      std::cout << "No execution mode specified."
                << "\n\n";
      displayHelp("", "");
      return Application::EXIT_USAGE;
   }

  private:
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

      const silo_api::DatabaseWatcher watcher(data_directory, database_mutex);

      Poco::Net::HTTPServer server(
         new silo_api::SiloRequestHandlerFactory(database_mutex),
         server_socket,
         new Poco::Net::HTTPServerParams
      );

      SPDLOG_INFO("Listening on port {}", port);

      server.start();
      waitForTerminationRequest();
      server.stop();

      return Application::EXIT_OK;
   };

   int handlePreprocessing() {
      SPDLOG_INFO("Starting SILO preprocessing");
      const auto preprocessing_config = preprocessingConfig(config());
      auto database_config = databaseConfig(config());

      auto database = silo::Database::preprocessing(preprocessing_config, database_config);

      database.saveDatabaseState(preprocessing_config.getOutputDirectory());

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
