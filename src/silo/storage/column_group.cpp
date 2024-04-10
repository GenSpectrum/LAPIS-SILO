#include "silo/storage/column_group.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <boost/algorithm/string.hpp>
#include <duckdb.hpp>

#include "silo/common/date.h"
#include "silo/common/json_value_type.h"
#include "silo/common/optional_bool.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/insertion_column.h"

namespace silo {
class AminoAcid;
class Nucleotide;
}  // namespace silo

namespace silo::storage {
using silo::config::ColumnType;

using silo::common::OptionalBool;

uint32_t ColumnPartitionGroup::fill(
   duckdb::Connection& connection,
   uint32_t partition_id,
   const std::string& order_by_clause,
   const silo::config::DatabaseConfig& database_config
) {
   uint32_t sequence_count = 0;

   std::vector<std::string> column_names;
   column_names.reserve(database_config.schema.metadata.size());
   for (const auto& item : database_config.schema.metadata) {
      column_names.push_back("\"" + item.name + "\"");
   }
   std::string column_name_sql = boost::algorithm::join(column_names, ", ");

   auto result = connection.Query(fmt::format(
      "SELECT {} FROM partitioned_metadata WHERE partition_id = {} {}",
      column_name_sql,
      partition_id,
      order_by_clause
   ));
   if (result->HasError()) {
      throw preprocessing::PreprocessingException(
         "Error in the execution of the duckdb statement for partition key table "
         "generation: " +
         result->GetError()
      );
   }
   const size_t row_count = result->RowCount();
   for (const auto& item : database_config.schema.metadata) {
      const auto column_type = item.getColumnType();
      reserveSpaceInColumn(item.name, column_type, row_count);
   }

   for (auto it = result->begin(); it != result->end(); ++it) {
      size_t column_index = 0;
      for (const auto& item : database_config.schema.metadata) {
         const auto column_type = item.getColumnType();
         const auto value = it.current_row.GetValue<duckdb::Value>(column_index++);
         addValueToColumn(item.name, column_type, value);
      }
      if (++sequence_count == UINT32_MAX) {
         throw std::runtime_error(
            "SILO is currently limited to UINT32_MAX=" + std::to_string(UINT32_MAX) + " sequences."
         );
      }
   }

   return sequence_count;
}

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
      case ColumnType::INDEXED_PANGOLINEAGE:
         pango_lineage_columns.at(column_name).insert({value.ToString()});
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
      case ColumnType::NUC_INSERTION:
         nuc_insertion_columns.at(column_name).insert(value.ToString());
         return;
      case ColumnType::AA_INSERTION:
         aa_insertion_columns.at(column_name).insert(value.ToString());
         return;
   }
}

void ColumnPartitionGroup::addNullToColumn(const std::string& column_name, ColumnType column_type) {
   switch (column_type) {
      case ColumnType::INDEXED_STRING:
         indexed_string_columns.at(column_name).insertNull();
         return;
      case ColumnType::STRING:
         string_columns.at(column_name).insertNull();
         return;
      case ColumnType::INDEXED_PANGOLINEAGE:
         pango_lineage_columns.at(column_name).insertNull();
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
      case ColumnType::NUC_INSERTION:
         nuc_insertion_columns.at(column_name).insertNull();
         return;
      case ColumnType::AA_INSERTION:
         aa_insertion_columns.at(column_name).insertNull();
         return;
   }
   abort();
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
      case ColumnType::INDEXED_PANGOLINEAGE:
         pango_lineage_columns.at(column_name).reserve(row_count);
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
      case ColumnType::NUC_INSERTION:
         nuc_insertion_columns.at(column_name).reserve(row_count);
         return;
      case ColumnType::AA_INSERTION:
         aa_insertion_columns.at(column_name).reserve(row_count);
         return;
   }
   abort();
}

template <>
const std::map<std::string, storage::column::InsertionColumnPartition<Nucleotide>&>&
ColumnPartitionGroup::getInsertionColumns<Nucleotide>() const {
   return this->nuc_insertion_columns;
}

template <>
const std::map<std::string, storage::column::InsertionColumnPartition<AminoAcid>&>&
ColumnPartitionGroup::getInsertionColumns<AminoAcid>() const {
   return this->aa_insertion_columns;
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
            case ColumnType::INDEXED_PANGOLINEAGE:
               result.pango_lineage_columns.insert({item.name, pango_lineage_columns.at(item.name)}
               );
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
            case ColumnType::NUC_INSERTION:
               result.nuc_insertion_columns.insert({item.name, nuc_insertion_columns.at(item.name)}
               );
               return;
            case ColumnType::AA_INSERTION:
               result.aa_insertion_columns.insert({item.name, aa_insertion_columns.at(item.name)});
               return;
         }
         abort();
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
   if (pango_lineage_columns.contains(column)) {
      return pango_lineage_columns.at(column)
         .lookupAliasedValue(pango_lineage_columns.at(column).getValues().at(sequence_id))
         .value;
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
      if (value == INT32_MIN) {
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
   if (nuc_insertion_columns.contains(column)) {
      return nuc_insertion_columns.at(column).lookupValue(
         nuc_insertion_columns.at(column).getValues().at(sequence_id)
      );
   }
   if (aa_insertion_columns.contains(column)) {
      return aa_insertion_columns.at(column).lookupValue(
         aa_insertion_columns.at(column).getValues().at(sequence_id)
      );
   }
   return std::nullopt;
}

template <>
const std::map<std::string, storage::column::InsertionColumn<Nucleotide>>& ColumnGroup::
   getInsertionColumns<Nucleotide>() const {
   return nuc_insertion_columns;
}
template <>
const std::map<std::string, storage::column::InsertionColumn<AminoAcid>>& ColumnGroup::
   getInsertionColumns<AminoAcid>() const {
   return aa_insertion_columns;
}

}  // namespace silo::storage
