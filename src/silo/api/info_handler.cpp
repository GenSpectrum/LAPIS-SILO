#include "silo/api/info_handler.h"

#include <map>
#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database_info.h"

namespace silo::api {

InfoHandler::InfoHandler(std::shared_ptr<ActiveDatabase> database_handle)
    : database_handle(database_handle) {}

void InfoHandler::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto database = database_handle->getActiveDatabase();

   response.set("data-version", database->getDataVersionTimestamp().value);

   const nlohmann::json database_info = nlohmann::json(database->getDatabaseInfo());
   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << database_info;
}
}  // namespace silo::api
