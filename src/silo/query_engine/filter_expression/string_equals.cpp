#include "silo/query_engine/filter_expressions/string_equals.h"

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

StringEquals::StringEquals(std::string column, std::string value)
    : column(std::move(column)),
      value(std::move(value)) {}

std::string StringEquals::toString(const silo::Database& /*database*/) {
   return column + " = '" + value + "'";
}

std::unique_ptr<silo::query_engine::operators::Operator> StringEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   /// TODO remove hardcoded columns and replace with a check for a precomputed bitmap index
   if (column == "country") {
      const std::optional<uint64_t> value_id = database.dict->getCountryIdInLookup(value);
      if (value_id.has_value()) {
         return std::make_unique<operators::IndexScan>(
            &database_partition.meta_store.country_bitmaps[value_id.value()]
         );
      }
      return std::make_unique<operators::Empty>();
   }
   if (column == "region") {
      const std::optional<uint64_t> value_id = database.dict->getRegionIdInLookup(value);
      if (value_id.has_value()) {
         return std::make_unique<operators::IndexScan>(
            &database_partition.meta_store.region_bitmaps[value_id.value()]
         );
      }
      return std::make_unique<operators::Empty>();
   }
   const std::optional<uint32_t> column_id = database.dict->getColumnIdInLookup(column);
   const std::optional<uint64_t> value_id = database.dict->getIdInGeneralLookup(value);
   if (column_id.has_value() && value_id.has_value()) {
      return std::make_unique<operators::Selection>(
         database_partition.meta_store.columns[column_id.value()].data(),
         database_partition.meta_store.columns[column_id.value()].size(),
         operators::Selection::EQUALS,
         value_id.value()
      );
   }
   return std::make_unique<operators::Empty>();
}

}  // namespace silo::query_engine::filter_expressions