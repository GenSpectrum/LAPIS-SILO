#ifndef SILO_QUERYREQUESTHANDLER_H
#define SILO_QUERYREQUESTHANDLER_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace silo {
class Database;
}

namespace silo_api {
class QueryHandler : public Poco::Net::HTTPRequestHandler {
  private:
   silo::Database& database;

  public:
   explicit QueryHandler(silo::Database& database);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};
}  // namespace silo_api

#endif  // SILO_QUERYREQUESTHANDLER_H
