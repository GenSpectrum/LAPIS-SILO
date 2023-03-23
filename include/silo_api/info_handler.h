#ifndef SILO_INFOHANDLER_H
#define SILO_INFOHANDLER_H

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "silo_api/request_handler.h"

namespace silo {
class Database;
}

namespace silo_api {
class InfoHandler : public RestResource {
  private:
   const silo::Database& database;

  public:
   explicit InfoHandler(const silo::Database& database);

   void get(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};
}  // namespace silo_api

#endif  // SILO_INFOHANDLER_H
