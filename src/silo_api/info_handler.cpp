#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "silo/database.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <silo_api/info_handler.h>

namespace silo {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(db_info_t, sequenceCount, totalSize, nBitmapsSize);
}

namespace silo_api {
void InfoHandler::handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response) {
   const auto db_info = database.get_db_info();

   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(db_info);
}
}
