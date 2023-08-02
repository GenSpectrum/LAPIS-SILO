#ifndef SILO_QUERYREQUESTHANDLER_H
#define SILO_QUERYREQUESTHANDLER_H

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "silo_api/rest_resource.h"

namespace silo::query_engine {
class QueryEngine;
}

namespace silo_api {
class QueryHandler : public RestResource {
  private:
   const silo::query_engine::QueryEngine& query_engine;
   const std::string data_version;

  public:
   explicit QueryHandler(
      const silo::query_engine::QueryEngine& query_engine,
      std::string data_version
   );

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo_api

#endif  // SILO_QUERYREQUESTHANDLER_H
