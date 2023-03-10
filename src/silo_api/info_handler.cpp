#include <silo_api/info_handler.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "silo/database.h"

namespace silo {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DatabaseInfo, sequenceCount, totalSize, nBitmapsSize);
}

namespace silo_api {
void InfoHandler::handleRequest(
   Poco::Net::HTTPServerRequest& /*request*/,
   Poco::Net::HTTPServerResponse& response
) {
   const auto db_info = database.getDatabaseInfo();

   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(db_info);
}
InfoHandler::InfoHandler(silo::Database& database)
    : database(database) {}
}  // namespace silo_api
