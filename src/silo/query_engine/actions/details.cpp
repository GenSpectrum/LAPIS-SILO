#include "silo/query_engine/actions/details.h"

#include <algorithm>
#include <random>
#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/actions/tuple.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column_group.h"

namespace silo::query_engine::actions {
Details::Details(std::vector<std::string> fields)
    : fields(std::move(fields)) {}

std::vector<schema::ColumnIdentifier> Details::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::vector<silo::schema::ColumnIdentifier> field_metadata;
   if (fields.empty()) {
      for (const auto& column : table_schema.getColumnIdentifiers()) {
         if (!isSequenceColumn(column.type)) {
            field_metadata.push_back(column);
         }
      }
      return field_metadata;
   }
   field_metadata.reserve(fields.size());
   for (const auto& field : fields) {
      auto column = table_schema.getColumn(field);
      CHECK_SILO_QUERY(column.has_value(), "Metadata field {} not found.", field);
      CHECK_SILO_QUERY(
         !isSequenceColumn(column.value().type),
         "The Details action does not support sequence-type columns for now."
      );
      field_metadata.push_back(column.value());
   }
   return field_metadata;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action) {
   const std::vector<std::string> fields = json.value("fields", std::vector<std::string>());
   action = std::make_unique<Details>(fields);
}

}  // namespace silo::query_engine::actions
