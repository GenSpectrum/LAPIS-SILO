#include "silo/api/lineage_definition_handler.h"

#include <map>
#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"
#include "silo/api/error_request_handler.h"

namespace silo::api {

LineageDefinitionHandler::LineageDefinitionHandler(
   std::shared_ptr<ActiveDatabase> database_handle,
   std::string column_name
)
    : database_handle(database_handle),
      column_name(std::move(column_name)) {}

void LineageDefinitionHandler::get(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto database = database_handle->getActiveDatabase();

   response.set("data-version", database->getDataVersionTimestamp().value);

   auto column_identifier = database->table->schema.getColumn(column_name);
   if (column_identifier == std::nullopt) {
      response.setContentType("application/json");
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         .error = "Bad request",
         .message = fmt::format("The column {} does not exist in this instance.", column_name)
      });
      return;
   }
   if (column_identifier.value().type != schema::ColumnType::INDEXED_STRING) {
      response.setContentType("application/json");
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         .error = "Bad request",
         .message = fmt::format("The column {} is not of type indexed-string.", column_name)
      });
      return;
   }
   auto metadata = database->table->schema
                      .getColumnMetadata<storage::column::IndexedStringColumnPartition>(column_name)
                      .value();
   if (!metadata->lineage_tree.has_value()) {
      response.setContentType("application/json");
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{
         .error = "Bad request",
         .message = fmt::format("The column {} does not have a lineageIndex defined.", column_name)
      });
      return;
   }
   const std::string lineage_definition_yaml = metadata->lineage_tree.value().file;
   response.setContentType("application/yaml");
   std::ostream& out_stream = response.send();
   out_stream << lineage_definition_yaml;
}
}  // namespace silo::api
