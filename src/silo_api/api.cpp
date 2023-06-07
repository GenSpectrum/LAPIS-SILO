#include <iostream>
#include <string>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <spdlog/spdlog.h>

#include "silo/config/config_repository.h"
#include "silo/database.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/query_engine/query_engine.h"
#include "silo_api/info_handler.h"
#include "silo_api/logging.h"
#include "silo_api/request_handler_factory.h"

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

      options.addOption(
         Poco::Util::Option("api", "a", "start the SILO web interface")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleApi))
      );

      options.addOption(
         Poco::Util::Option(
            "processData",
            "p",
            "trigger the preprocessing pipeline to generate a partitioned dataset that can be read "
            "by the database"
         )
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleProcessData))
      );
   }

   int main(const std::vector<std::string>& args) override {
      if (!args.empty()) {
         displayHelp("", "");
         return Application::EXIT_USAGE;
      }

      return Application::EXIT_OK;
   }

  private:
   void handleApi(
      [[maybe_unused]] const std::string& name,
      [[maybe_unused]] const std::string& value
   ) {
      int const port = 8081;

      const std::string preprocessing_config_path = config().getString("preprocessingConfig");
      const std::string database_config_path = config().getString("databaseConfig");

      auto preprocessing_config =
         silo::preprocessing::PreprocessingConfigReader().readConfig(preprocessing_config_path);
      auto database_config =
         silo::config::ConfigRepository().getValidatedConfig(database_config_path);

      auto database = silo::Database(preprocessing_config.input_directory);

      database.preprocessing(preprocessing_config, database_config);

      Poco::Net::ServerSocket const server_socket(port);
      const silo::QueryEngine query_engine = silo::QueryEngine(database);
      Poco::Net::HTTPServer server(
         new silo_api::SiloRequestHandlerFactory(database, query_engine),
         server_socket,
         new Poco::Net::HTTPServerParams
      );

      SPDLOG_INFO("Listening on port {}", port);

      server.start();
      waitForTerminationRequest();
      server.stop();
   };

   // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
   void handleProcessData(
      [[maybe_unused]] const std::string& name,
      [[maybe_unused]] const std::string& value
   ) {
      std::cout << "handleProcessData is not implemented\n" << std::flush;
   };

   void displayHelp(
      [[maybe_unused]] const std::string& name,
      [[maybe_unused]] const std::string& value
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
