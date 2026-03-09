#include "silo/query_engine/actions/order_by_field.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine {

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void from_json(const nlohmann::json& json, OrderByField& field) {
   if (json.is_string()) {
      field = {.name = json.get<std::string>(), .ascending = true};
      return;
   }
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("field") && json.contains("order") &&
         json["field"].is_string() && json["order"].is_string(),
      "The orderByField '{}' must be either a string or an object containing the fields "
      "'field':string and 'order':string, where the value of order is 'ascending' or 'descending'",
      json.dump()
   );
   const std::string field_name = json["field"].get<std::string>();
   const std::string order_string = json["order"].get<std::string>();
   CHECK_SILO_QUERY(
      order_string == "ascending" || order_string == "descending",
      "The orderByField '{}' must be either a string or an object containing the fields "
      "'field':string and 'order':string, where the value of order is 'ascending' or 'descending'",
      json.dump()
   );
   field = {.name = field_name, .ascending = order_string == "ascending"};
}

}  // namespace silo::query_engine
