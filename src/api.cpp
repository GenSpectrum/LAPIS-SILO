
#include "../include/silo/database.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include <iostream>
#include <vector>
#include <silo/database.h>
#include <silo/query_engine/query_engine.h>

using namespace silo;

//int handle_command(Database& db, std::vector<std::string> args) {
//   if ("info" == args[0]) {
//      if (args.size() > 1) {
//         std::ofstream out(args[1]);
//         if (!out) {
//            std::cout << "Could not open outfile " << args[1] << endl;
//            return 0;
//         }
//         db.db_info(out);
//      } else {
//         db.db_info(std::cout);
//      }
//   } else if ("query" == args[0]) {
//      if (args.size() < 2) {
//         std::cout << "Expected syntax: \"query <JSON_QUERY> [query_dir]\"" << endl;
//         return 0;
//      }
//      std::string test_name = args[1];
//
//      std::string query_dir_str = args.size() > 2 ? args[2] : default_query_dir;
//
//      std::ifstream query_file(query_dir_str + test_name);
//      if (!query_file || !query_file.good()) {
//         std::cerr << "query_file " << (query_dir_str + test_name) << " not found." << std::endl;
//         return 0;
//      }
//
//      std::stringstream buffer;
//      buffer << query_file.rdbuf();
//
//      std::string query = "{\"action\": {\"type\": \"Aggregated\"" /*,\"groupByFields\": [\"date\",\"division\"]*/ "},\"filter\": " + buffer.str() + "}";
//      execute_query(db, query, std::cout, std::cout, std::cout);
//      query = "{\"action\": {\"type\": \"Mutations\"},\"filter\": " + buffer.str() + "}";
//      execute_query(db, query, std::cout, std::cout, std::cout);
//   }
//}

class QueryRequestHandler : public Poco::Net::HTTPRequestHandler {
   private:
   std::shared_ptr<silo::Database> database{};

   public:
   explicit QueryRequestHandler(const std::shared_ptr<silo::Database>& database) : database(database) {
   }

   void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) override {
      std::ostream& out_stream = response.send();
      out_stream << "TODO query";
   }
};

class InfoHandler : public Poco::Net::HTTPRequestHandler {
   private:
   std::shared_ptr<silo::Database> database{};

   public:
   explicit InfoHandler(const std::shared_ptr<silo::Database>& database) : database(database) {
   }

   void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) override {
      response.setContentType("text/text");
      std::ostream& out_stream = response.send();
      out_stream << "TODO info";
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
         return nullptr;
   }
};

class SiloServer : public Poco::Util::ServerApplication {
   public:
   protected:
   //   void initialize(Application& self) override {
   //      loadConfiguration(); // load default configuration files, if present
   //      ServerApplication::initialize(self);
   //   }
   //
   //   void uninitialize() override {
   //      ServerApplication::uninitialize();
   //   }

   //   void defineOptions(OptionSet& options) {
   //      ServerApplication::defineOptions(options);
   //
   //      options.addOption(
   //         Option("help", "h", "display help information on command line arguments")
   //            .required(false)
   //            .repeatable(false));
   //   }
   //
   //   void handleOption(const std::string& name, const std::string& value) {
   //      ServerApplication::handleOption(name, value);
   //
   //      if (name == "help")
   //         _helpRequested = true;
   //   }

   void displayHelp() {
      //      HelpFormatter helpFormatter(options());
      //      helpFormatter.setCommand(commandName());
      //      helpFormatter.setUsage("OPTIONS");
      //      helpFormatter.setHeader("A web server that serves the current date and time.");
      //      helpFormatter.format(std::cout);
   }

   int main(const std::vector<std::string>& args) override {
      if (args.empty() || args.at(0) != "api") {
         return Application::EXIT_OK;
      }
      const char* working_directory = "./";
      auto database = std::make_shared<silo::Database>(working_directory);

      //      int maxQueued = config().getInt("SiloServer.maxQueued", 100);
      //      int maxThreads = config().getInt("SiloServer.maxThreads", 16);
      //      Poco::ThreadPool::defaultPool().addCapacity(maxThreads);

      auto* parameters = new Poco::Net::HTTPServerParams;
      //      parameters->setMaxQueued(maxQueued);
      //      parameters->setMaxThreads(maxThreads);

      Poco::Net::ServerSocket server_socket(8080);
      Poco::Net::HTTPServer server(new SiloRequestHandlerFactory(database), server_socket, parameters);
      server.start();
      waitForTerminationRequest();
      server.stop();
      return Application::EXIT_OK;
   }
};

int main(int argc, char** argv) {
   SiloServer app;
   return app.run(argc, argv);
}
