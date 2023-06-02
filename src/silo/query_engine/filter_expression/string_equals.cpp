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
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition
) const {
   if (database_partition.meta_store.indexed_string_columns.contains(column)) {
      const auto& string_column = database_partition.meta_store.indexed_string_columns.at(column);
      const roaring::Roaring& bitmap = string_column.filter(value);

      if (bitmap.isEmpty()) {
         return std::make_unique<operators::Empty>(database_partition.sequenceCount);
      }
      return std::make_unique<operators::IndexScan>(
         new roaring::Roaring(bitmap), database_partition.sequenceCount
      );
   }

   if (database_partition.meta_store.raw_string_columns.contains(column)) {
      const auto& string_column = database_partition.meta_store.raw_string_columns.at(column);

      return std::make_unique<operators::Selection<std::string>>(
         string_column.getValues(),
         operators::Selection<std::string>::EQUALS,
         value,
         database_partition.sequenceCount
      );
   }

   return std::make_unique<operators::Empty>(database_partition.sequenceCount);
}

}  // namespace silo::query_engine::filter_expressions