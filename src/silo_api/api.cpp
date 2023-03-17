#include <iostream>
#include <string>
#include <vector>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>

#include "silo/database.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo_api/info_handler.h"
#include "silo_api/not_found_handler.h"
#include "silo_api/query_handler.h"

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   silo::Database& database;

  public:
   explicit SiloRequestHandlerFactory(silo::Database& database)
       : database(database) {}

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request
   ) override {
      if (request.getURI() == "/info") {
         return new silo_api::InfoHandler(database);
      }
      if (request.getURI() == "/query") {
         return new silo_api::QueryHandler(database);
      }
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
   void handleApi(
      [[maybe_unused]] const std::string& name,
      [[maybe_unused]] const std::string& value
   ) {
      int const port = 8080;

      const silo::InputDirectory input_directory{"./"};
      const silo::OutputDirectory output_directory{"./"};
      const silo::MetadataFilename metadata_filename{"small_metadata_set.tsv"};
      const silo::SequenceFilename sequence_filename{"small_sequence_set.fasta"};
      auto config = silo::PreprocessingConfig(
         input_directory, output_directory, metadata_filename, sequence_filename
      );

      auto database = silo::Database(input_directory.directory);

      database.preprocessing(config);
      std::cout << "finished preprocessing " << std::endl;

      Poco::Net::ServerSocket const server_socket(port);
      Poco::Net::HTTPServer server(
         new SiloRequestHandlerFactory(database), server_socket, new Poco::Net::HTTPServerParams
      );

      std::cout << "listening on port " << port << std::endl;

      server.start();
      waitForTerminationRequest();
      server.stop();
   };

   // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
   void handleProcessData(
      [[maybe_unused]] const std::string& name,
      [[maybe_unused]] const std::string& value
   ) {
      std::cout << "handleProcessData is not implemented" << std::endl;
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
   SiloServer app;
   return app.run(argc, argv);
}
