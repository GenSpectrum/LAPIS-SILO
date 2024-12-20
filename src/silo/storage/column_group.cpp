#include "silo/storage/column_group.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <duckdb.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/date.h"
#include "silo/common/json_value_type.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/optional_bool.h"
#include "silo/common/panic.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage {
using silo::config::ColumnType;

using silo::common::OptionalBool;

void ColumnPartitionGroup::addValueToColumn(
   const std::string& column_name,
   ColumnType column_type,
   const duckdb::Value& value
) {
   if (value.IsNull()) {
      addNullToColumn(column_name, column_type);
      return;
   }
   switch (column_type) {
      case ColumnType::INDEXED_STRING:
         indexed_string_columns.at(column_name).insert(value.ToString());
         return;
      case ColumnType::STRING:
         string_columns.at(column_name).insert(value.ToString());
         return;
      case ColumnType::DATE:
         date_columns.at(column_name).insert(common::stringToDate(value.ToString()));
         return;
      case ColumnType::BOOL:
         if (value.type() != duckdb::LogicalType::BOOLEAN) {
            auto str = value.ToString();
            throw silo::preprocessing::PreprocessingException(
               fmt::format("trying to insert the value '{}' into column '{}'", str, column_name)
            );
         } else {
            bool_columns.at(column_name).insert(duckdb::BooleanValue::Get(value));
         }
         return;
      case ColumnType::INT:
         int_columns.at(column_name).insert(value.ToString());
         return;
      case ColumnType::FLOAT:
         float_columns.at(column_name).insert(value.ToString());
         return;
   }
   SILO_UNREACHABLE();
}

void ColumnPartitionGroup::addNullToColumn(const std::string& column_name, ColumnType column_type) {
   switch (column_type) {
      case ColumnType::INDEXED_STRING:
         indexed_string_columns.at(column_name).insertNull();
         return;
      case ColumnType::STRING:
         string_columns.at(column_name).insertNull();
         return;
      case ColumnType::DATE:
         date_columns.at(column_name).insertNull();
         return;
      case ColumnType::BOOL:
         bool_columns.at(column_name).insertNull();
         return;
      case ColumnType::INT:
         int_columns.at(column_name).insertNull();
         return;
      case ColumnType::FLOAT:
         float_columns.at(column_name).insertNull();
         return;
   }
   SILO_UNREACHABLE();
}

void ColumnPartitionGroup::reserveSpaceInColumn(
   const std::string& column_name,
   ColumnType column_type,
   size_t row_count
) {
   switch (column_type) {
      case ColumnType::INDEXED_STRING:
         indexed_string_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::STRING:
         string_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::DATE:
         date_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::BOOL:
         bool_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::INT:
         int_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::FLOAT:
         float_columns.at(column_name).reserve(row_count);
         return;
   }
   SILO_UNREACHABLE();
}

ColumnPartitionGroup ColumnPartitionGroup::getSubgroup(
   const std::vector<silo::storage::ColumnMetadata>& fields
) const {
   ColumnPartitionGroup result;
   for (const auto& field : fields) {
      result.metadata.push_back({field.name, field.type});
   }

   for (const auto& item : fields) {
      ([&]() {
         switch (item.type) {
            case ColumnType::INDEXED_STRING:
               result.indexed_string_columns.insert(
                  {item.name, indexed_string_columns.at(item.name)}
               );
               return;
            case ColumnType::STRING:
               result.string_columns.insert({item.name, string_columns.at(item.name)});
               return;
            case ColumnType::DATE:
               result.date_columns.insert({item.name, date_columns.at(item.name)});
               return;
            case ColumnType::BOOL:
               result.bool_columns.insert({item.name, bool_columns.at(item.name)});
               return;
            case ColumnType::INT:
               result.int_columns.insert({item.name, int_columns.at(item.name)});
               return;
            case ColumnType::FLOAT:
               result.float_columns.insert({item.name, float_columns.at(item.name)});
               return;
         }
         SILO_UNREACHABLE();
      })();
   }
   return result;
}

common::JsonValueType ColumnPartitionGroup::getValue(
   const std::string& column,
   uint32_t sequence_id
) const {
   if (string_columns.contains(column)) {
      return string_columns.at(column).lookupValue(
         string_columns.at(column).getValues().at(sequence_id)
      );
   }
   if (indexed_string_columns.contains(column)) {
      return indexed_string_columns.at(column).lookupValue(
         indexed_string_columns.at(column).getValues().at(sequence_id)
      );
   }
   if (date_columns.contains(column)) {
      return common::dateToString(date_columns.at(column).getValues().at(sequence_id));
   }
   if (bool_columns.contains(column)) {
      const OptionalBool value = bool_columns.at(column).getValues().at(sequence_id);
      if (value.isNull()) {
         return std::nullopt;
      }
      return value.value();
   }
   if (int_columns.contains(column)) {
      int32_t value = int_columns.at(column).getValues().at(sequence_id);
      if (value == column::IntColumn::null()) {
         return std::nullopt;
      }
      return value;
   }
   if (float_columns.contains(column)) {
      double value = float_columns.at(column).getValues().at(sequence_id);
      if (value == std::nan("")) {
         return std::nullopt;
      }
      return value;
   }
   return std::nullopt;
}

}  // namespace silo::storage
