#include <crow.h>

#include "silo/api/lineage_definition_handler.h"

#include <string>

#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"

namespace silo::api {

void LineageDefinitionHandler::get(
   std::shared_ptr<const Database> database,
   crow::request& /*request*/,
   crow::response& response,
   const std::string& column_name
) {
   response.set_header("data-version", database->getDataVersionTimestamp().value);

   auto column_metadata = std::ranges::find_if(
      database->columns.metadata, [&](const auto& metadata) { return metadata.name == column_name; }
   );
   if (column_metadata == database->columns.metadata.end()) {
      response.set_header("Content-Type", "application/json");
      response.code = crow::BAD_REQUEST;
      response.body = nlohmann::json{
         {"error", "Bad request"},
         {"message", fmt::format("The column {} does not exist in this instance.", column_name)}
      }.dump();
      response.end();
   }
   // TODO(#691) Change this check for containment to a selection of the correct lineage system
   else if(column_metadata->type != config::ColumnType::INDEXED_STRING || !database->columns.indexed_string_columns.at(column_name).hasLineageTree()){
      response.set_header("Content-Type", "application/json");
      response.code = crow::BAD_REQUEST;
      response.body = nlohmann::json{
         {"error", "Bad request"},
         {"message", fmt::format("The column {} does not have a lineageIndex defined.", column_name)
         }
      }.dump();
      response.end();
   } else {
      const std::string lineage_definition_yaml = database->lineage_tree.file;
      response.set_header("Content-Type", "application/yaml");
      response.body = lineage_definition_yaml;
      response.end();
   }
}
}  // namespace silo::api
