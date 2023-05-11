#include "silo/query_engine/filter_expressions/string_equals.h"

#include <utility>
#include <vector>

#include "silo/database.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/storage/database_partition.h"

namespace operators = silo::query_engine::operators;

namespace silo::query_engine::filter_expressions {

StringEquals::StringEquals(std::string column, std::string value)
    : column(std::move(column)),
      value(std::move(value)) {}

std::string StringEquals::toString(const silo::Database& /*database*/) {
   std::string res = column;
   res += " = '";
   res += value;
   res += "'";
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> StringEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   /// TODO remove hardcoded columns and replace with a check for a precomputed bitmap index
   if (column == "country") {
      const uint64_t value_id = database.dict->getCountryIdInLookup(value);
      return std::make_unique<operators::IndexScan>(
         &database_partition.meta_store.country_bitmaps[value_id]
      );
   }
   if (column == "region") {
      const uint64_t value_id = database.dict->getRegionIdInLookup(value);
      return std::make_unique<operators::IndexScan>(
         &database_partition.meta_store.region_bitmaps[value_id]
      );
   }
   const uint32_t column_id = database.dict->getColumnIdInLookup(column);
   const uint64_t value_id = database.dict->getIdInGeneralLookup(value);
   return std::make_unique<operators::Selection>(
      database_partition.meta_store.columns[column_id].data(),
      operators::Selection::EQUALS,
      value_id,
      database_partition.sequenceCount
   );
}

}  // namespace silo::query_engine::filter_expressions