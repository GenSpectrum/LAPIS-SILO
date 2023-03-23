#include "silo_api/info_handler.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"

namespace silo {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DatabaseInfo, sequenceCount, totalSize, nBitmapsSize);
}

namespace silo_api {

InfoHandler::InfoHandler(const silo::Database& database)
    : database(database) {}

void InfoHandler::get(
   Poco::Net::HTTPServerRequest& /*request*/,
   Poco::Net::HTTPServerResponse& response
) {
   const auto db_info = database.getDatabaseInfo();

   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(db_info);
}

}  // namespace silo_api
