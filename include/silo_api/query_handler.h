#ifndef SILO_QUERYREQUESTHANDLER_H
#define SILO_QUERYREQUESTHANDLER_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace silo {
class QueryEngine;
}

namespace silo_api {
class QueryHandler : public Poco::Net::HTTPRequestHandler {
  private:
 const  silo::QueryEngine& query_engine;

  public:
   explicit QueryHandler(const silo::QueryEngine& query_engine);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};
}  // namespace silo_api

#endif  // SILO_QUERYREQUESTHANDLER_H
