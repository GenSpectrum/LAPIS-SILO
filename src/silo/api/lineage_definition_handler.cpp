#include "silo/api/lineage_definition_handler.h"

#include <map>
#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
#include <nlohmann/json.hpp>

#include "silo/api/database_mutex.h"
#include "silo/api/error_request_handler.h"

namespace silo::api {

LineageDefinitionHandler::LineageDefinitionHandler(DatabaseMutex& database, std::string column_name)
    : database(database),
      column_name(std::move(column_name)) {}

void LineageDefinitionHandler::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto fixed_database = database.getDatabase();

   response.set("data-version", fixed_database->getDataVersionTimestamp().value);

   auto column_metadata =
      std::ranges::find_if(fixed_database->columns.metadata, [&](const auto& metadata) {
         return metadata.name == column_name;
      });
   if (column_metadata == fixed_database->columns.metadata.end()) {
      response.setContentType("application/json");
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         .error = "Bad request",
         .message = fmt::format("The column {} does not exist in this instance.", column_name)
      });
   }
   // TODO(#691) Change this check for containment to a selection of the correct lineage system
   else if(column_metadata->type != config::ColumnType::INDEXED_STRING || !fixed_database->columns.indexed_string_columns.at(column_name).hasLineageTree()){
      response.setContentType("application/json");
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         .error = "Bad request",
         .message = fmt::format("The column {} does not have a lineageIndex defined.", column_name)
      });
   } else {
      const std::string lineage_definition_yaml = fixed_database->lineage_tree.file;
      response.setContentType("application/yaml");
      std::ostream& out_stream = response.send();
      out_stream << lineage_definition_yaml;
   }
}
}  // namespace silo::api
