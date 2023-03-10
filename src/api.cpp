#include <silo/preprocessing/preprocessing_config.h>
#include <silo_api/error.h>
#include <silo_api/info_handler.h>
#include <silo_api/query_handler.h>
#include <iostream>
#include <vector>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "silo/database.h"

using namespace silo;

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   silo::Database& database;

  public:
   explicit SiloRequestHandlerFactory(silo::Database& database)
       : database(database) {}

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request
   ) override {
      if (request.getURI() == "/info")
         return new silo_api::InfoHandler(database);
      if (request.getURI() == "/query")
         return new silo_api::QueryHandler(database);
      else
         return new silo_api::NotFoundHandler;
   }
};

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
         Poco::Util::Option("api", "a", "start the SILO web interface")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleApi))
      );

      options.addOption(
         Poco::Util::Option(
            "processData", "p",
            "trigger the preprocessing pipeline to generate a partitioned dataset that can be read "
            "by the database"
         )
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleProcessData))
      );
   }

   int main(const std::vector<std::string>& args) override {
      if (args.empty()) {
         return Application::EXIT_OK;
      }

      std::cout << "Found unknown arguments:" << std::endl;
      for (const auto& arg : args) {
         std::cout << arg << std::endl;
      }

      displayHelp("", "");

      return Application::EXIT_USAGE;
   }

  private:
   void handleApi(const std::string&, const std::string&) {
      int port = 8080;

      const std::string input_directory("./");
      const std::string output_directory("./");
      auto config = silo::PreprocessingConfig(
         input_directory, output_directory, "minimal_metadata_set.tsv", "minimal_sequence_set.fasta"
      );

      auto database = silo::Database(input_directory);

      database.preprocessing(config);
      std::cout << "finished preprocessing " << std::endl;

      Poco::Net::ServerSocket server_socket(port);
      Poco::Net::HTTPServer server(
         new SiloRequestHandlerFactory(database), server_socket, new Poco::Net::HTTPServerParams
      );

      std::cout << "listening on port " << port << std::endl;

      server.start();
      waitForTerminationRequest();
      server.stop();
   };

   void handleProcessData(const std::string&, const std::string&) {
      std::cout << "handleProcessData is not implemented" << std::endl;
   };

   void displayHelp(const std::string&, const std::string&) {
      Poco::Util::HelpFormatter helpFormatter(options());
      helpFormatter.setCommand(commandName());
      helpFormatter.setUsage("OPTIONS");
      helpFormatter.setHeader("SILO - Sequence Indexing engine for Large Order of genomic data");
      helpFormatter.format(std::cout);
   }
};

int main(int argc, char** argv) {
   SiloServer app;
   return app.run(argc, argv);
}
