
#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/StreamCopier.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "silo/database.h"
#include "silo/query_engine/query_engine.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

using namespace silo;

struct error_response {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(error_response, error, message);

namespace silo {
   NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(db_info_t, sequence_count, total_size, N_bitmaps_size);
}

class QueryRequestHandler : public Poco::Net::HTTPRequestHandler {
   private:
   std::shared_ptr<silo::Database> database{};

   public:
   explicit QueryRequestHandler(const std::shared_ptr<silo::Database>& database) : database(database) {
   }

   void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override {
      std::string query;
      std::istream& istream = request.stream();
      Poco::StreamCopier::copyToString(istream, query);

      response.setContentType("application/json");

      try {
         const result_s& query_result = silo::execute_query(*database, query, std::cout, std::cout, std::cout);

         std::ostream& out_stream = response.send();
         Poco::JSON::Object output;
         output.set("result", query_result.return_message);
         output.stringify(out_stream);
      } catch (const silo::QueryParseException& ex) {
         response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
         std::ostream& out_stream = response.send();
         out_stream << nlohmann::json(error_response{"Bad request", ex.what()});
      } catch (const std::exception& ex) {
         response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
         std::ostream& out_stream = response.send();
         out_stream << nlohmann::json(error_response{"Internal server error", ex.what()});
      }
   }
};

class InfoHandler : public Poco::Net::HTTPRequestHandler {
   private:
   std::shared_ptr<silo::Database> database{};

   public:
   explicit InfoHandler(const std::shared_ptr<silo::Database>& database) : database(database) {
   }

   void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) override {
      const auto db_info = database->get_db_info();

      response.setContentType("application/json");
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(db_info);
   }
};

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
   void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) override {
      response.setContentType("application/json");
      response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(error_response{"Not found", "Resource does not exist"});
   }
};

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
   private:
   std::shared_ptr<silo::Database> database{};

   public:
   explicit SiloRequestHandlerFactory(const std::shared_ptr<silo::Database>& database) : database(database) {
   }

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override {
      if (request.getURI() == "/info")
         return new InfoHandler(database);
      if (request.getURI() == "/query")
         return new QueryRequestHandler(database);
      else
         return new NotFoundHandler;
   }
};

class SiloServer : public Poco::Util::ServerApplication {
   protected:
   void defineOptions(Poco::Util::OptionSet& options) override {
      ServerApplication::defineOptions(options);

      options.addOption(
         Poco::Util::Option("help", "h", "display help information on command line arguments")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::displayHelp)));

      options.addOption(
         Poco::Util::Option("api", "a", "start the SILO web interface")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleApi)));

      options.addOption(
         Poco::Util::Option(
            "processData",
            "p",
            "trigger the preprocessing pipeline to generate a partitioned dataset that can be read by the database")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<SiloServer>(this, &SiloServer::handleProcessData)));
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

      const char* working_directory = "./";
      auto database = std::make_shared<silo::Database>(working_directory);

      Poco::Net::ServerSocket server_socket(port);
      Poco::Net::HTTPServer server(new SiloRequestHandlerFactory(database), server_socket, new Poco::Net::HTTPServerParams);

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
