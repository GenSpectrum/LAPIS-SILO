#include "silo_api/info_handler.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/database_info.h"

namespace silo {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DatabaseInfo, sequenceCount, totalSize, nBitmapsSize);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
   BitmapContainerSizeStatistic,
   numberOfArrayContainers,
   numberOfRunContainers,
   numberOfBitsetContainers,
   numberOfValuesStoredInArrayContainers,
   numberOfValuesStoredInRunContainers,
   numberOfValuesStoredInBitsetContainers,
   totalBitmapSizeArrayContainers,
   totalBitmapSizeRunContainers,
   totalBitmapSizeBitsetContainers
);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BitmapSizePerSymbol, sizeInBytes);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
   BitmapContainerSize,
   sectionLength,
   sizePerGenomeSymbolAndSection,
   bitmapContainerSizeStatistic,
   totalBitmapSizeFrozen,
   totalBitmapSizeComputed
);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
   DetailedDatabaseInfo,
   bitmapSizePerSymbol,
   bitmapContainerSizePerGenomeSection
);
}  // namespace silo

namespace silo_api {

std::map<std::string, std::string> getQueryParameter(const Poco::Net::HTTPServerRequest& request) {
   std::map<std::string, std::string> map;
   const Poco::URI uri(request.getURI());
   const auto query_parameters = uri.getQueryParameters();

   for (const auto& parameter : query_parameters) {
      map.insert(parameter);
   }
   return map;
}

InfoHandler::InfoHandler(const silo::Database& database)
    : database(database) {}

void InfoHandler::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto request_parameter = getQueryParameter(request);

   if(request_parameter.find("details") != request_parameter.end() && request_parameter.at("details") == "true") {
      returnDetailedDatabaseInfo(response);
      return;
   }
   returnSimpleDatabaseInfo(response);
}

void InfoHandler::returnSimpleDatabaseInfo(Poco::Net::HTTPServerResponse& response) {
   const auto database_info = database.getDatabaseInfo();
   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(database_info);
}

void InfoHandler::returnDetailedDatabaseInfo(Poco::Net::HTTPServerResponse& response) {
   const auto detailed_info = database.detailedDatabaseInfo();
   response.setContentType("application/json");
   std::ostream& out_stream = response.send();
   out_stream << nlohmann::json(detailed_info);
}

}  // namespace silo_api
