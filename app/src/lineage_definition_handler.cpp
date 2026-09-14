#include "lineage_definition_handler.h"

#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>
#include <utility>

#include <rhydb/query_engine/illegal_query_exception.h>

#include "active_database.h"
#include "bad_request.h"
#include "error_request_handler.h"

namespace rhydb_app {

namespace {

/// The lineage definition file `column_name`'s values belong to, kept with the lineage relation
/// table that preprocessing built from it.
std::string getLineageDefinition(const rhydb::Database& database, const std::string& column_name) {
   const auto relation_table = database.tables.find(rhydb::schema::TableName{column_name});
   if (relation_table != database.tables.end() &&
       relation_table->second->schema->lineage_definition_file.has_value()) {
      return relation_table->second->schema->lineage_definition_file.value();
   }
   throw BadRequest("The column {} does not have a lineageIndex defined.", column_name);
}

}  // namespace

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

   const rhydb::schema::TableName& table_name = rhydb::schema::TableName::getDefault();

   auto table = database->tables.find(table_name);
   if (table == database->tables.end()) {
      throw BadRequest("The database does not contain a table with name {}", table_name.getName());
   }

   auto column_identifier = table->second->schema->getColumn(column_name);
   if (column_identifier == std::nullopt) {
      throw BadRequest("The column {} does not exist in this instance.", column_name);
   }
   if (column_identifier.value().type != rhydb::schema::ColumnType::DICTIONARY_ENCODED) {
      throw BadRequest("The column {} is not of type dictionary-encoded string.", column_name);
   }

   // Resolved before the response is sent: sending commits the 200, after which a BadRequest
   // could no longer set the status.
   const std::string lineage_definition_yaml = getLineageDefinition(*database, column_name);

   response.setContentType("application/yaml");
   std::ostream& out_stream = response.send();
   out_stream << lineage_definition_yaml;
}
}  // namespace rhydb_app
