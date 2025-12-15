#include "silo/api/lineage_definition_handler.h"

#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>
#include <utility>

#include "silo/api/active_database.h"
#include "silo/api/bad_request.h"
#include "silo/api/error_request_handler.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::api {

LineageDefinitionHandler::LineageDefinitionHandler(
   std::shared_ptr<ActiveDatabase> database_handle,
   std::string column_name
)
    : database_handle(std::move(database_handle)),
      column_name(std::move(column_name)) {}

void LineageDefinitionHandler::get(
   Poco::Net::HTTPServerRequest& /*request*/,
   Poco::Net::HTTPServerResponse& response
) {
   const auto database = database_handle->getActiveDatabase();

   response.set("data-version", database->getDataVersionTimestamp().value);

   schema::TableName table_name = schema::TableName::getDefault();

   auto table = database->tables.find(table_name);
   if (table == database->tables.end()) {
      throw BadRequest("The database does not contain a table with name {}", table_name.getName());
   }

   auto column_identifier = table->second->schema.getColumn(column_name);
   if (column_identifier == std::nullopt) {
      throw BadRequest("The column {} does not exist in this instance.", column_name);
   }
   if (column_identifier.value().type != schema::ColumnType::INDEXED_STRING) {
      throw BadRequest("The column {} is not of type indexed-string.", column_name);
   }
   auto* metadata =
      table->second->schema
         .getColumnMetadata<storage::column::IndexedStringColumnPartition>(column_name)
         .value();
   if (!metadata->lineage_tree.has_value()) {
      throw BadRequest("The column {} does not have a lineageIndex defined.", column_name);
   }
   const std::string lineage_definition_yaml = metadata->lineage_tree.value().file;
   response.setContentType("application/yaml");
   std::ostream& out_stream = response.send();
   out_stream << lineage_definition_yaml;
}
}  // namespace silo::api
