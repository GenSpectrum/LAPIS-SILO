#include "silo/query_engine/actions/details.h"

#include <nlohmann/json.hpp>

#include "silo/common/date.h"
#include "silo/common/string.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::actions {
Details::Details(std::vector<std::string> fields)
    : fields(std::move(fields)) {}

std::vector<config::DatabaseMetadata> parseFields(
   const silo::Database& database,
   const std::vector<std::string>& fields
) {
   if (fields.empty()) {
      return database.database_config.schema.metadata;
   }
   std::vector<config::DatabaseMetadata> field_metadata;
   for (const std::string& field : fields) {
      const auto& metadata = database.database_config.getMetadata(field);
      field_metadata.push_back(metadata);
   }
   return field_metadata;
}

QueryResult Details::execute(
   const silo::Database& database,
   std::vector<silo::query_engine::OperatorResult> bitmap_filter
) const {
   const std::vector<config::DatabaseMetadata> field_metadata = parseFields(database, fields);

   std::vector<QueryResultEntry> results;
   for (size_t partition_id = 0; partition_id < database.partitions.size(); partition_id++) {
      const auto& bitmap = bitmap_filter[partition_id];
      const auto& columns = database.partitions[partition_id].columns;
      for (const Idx sequence_id : *bitmap) {
         std::map<std::string, std::variant<std::string, int32_t, double>> row_fields;
         for (const auto& metadata : field_metadata) {
            if (metadata.getColumnType() == config::ColumnType::DATE) {
               const common::Date value =
                  columns.date_columns.at(metadata.name).getValues()[sequence_id];
               row_fields[metadata.name] = common::dateToString(value);
            } else if (metadata.getColumnType() == config::ColumnType::INT) {
               const int32_t value = columns.int_columns.at(metadata.name).getValues()[sequence_id];
               row_fields[metadata.name] = value;
            } else if (metadata.getColumnType() == config::ColumnType::FLOAT) {
               const double value =
                  columns.float_columns.at(metadata.name).getValues()[sequence_id];
               row_fields[metadata.name] = value;
            } else if (metadata.getColumnType() == config::ColumnType::STRING) {
               const auto& column = columns.string_columns.at(metadata.name);
               const common::String<common::STRING_SIZE> value = column.getValues()[sequence_id];
               row_fields[metadata.name] = column.lookupValue(value);
            } else if (metadata.getColumnType() == config::ColumnType::INDEXED_PANGOLINEAGE) {
               const auto& column = columns.pango_lineage_columns.at(metadata.name);
               const silo::Idx value = column.getValues()[sequence_id];
               row_fields[metadata.name] = column.lookupValue(value).value;
            } else if (metadata.getColumnType() == config::ColumnType::INDEXED_STRING) {
               const auto& column = columns.indexed_string_columns.at(metadata.name);
               const silo::Idx value = column.getValues()[sequence_id];
               row_fields[metadata.name] = column.lookupValue(value);
            } else {
               throw std::runtime_error("Unchecked column type of column " + metadata.name);
            }
         }
         results.push_back(QueryResultEntry{row_fields});
      }
   }
   return {results};
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action) {
   const std::vector<std::string> fields = json.value("fields", std::vector<std::string>());
   action = std::make_unique<Details>(fields);
}

}  // namespace silo::query_engine::actions