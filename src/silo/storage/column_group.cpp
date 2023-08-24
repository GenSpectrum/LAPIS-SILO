#include "silo/storage/column_group.h"

#include <cmath>
#include <csv.hpp>
#include <stdexcept>

#include "silo/common/date.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo::storage {

uint32_t ColumnPartitionGroup::fill(
   const std::filesystem::path& input_file,
   const silo::config::DatabaseConfig& database_config
) {
   auto metadata_reader = silo::preprocessing::MetadataReader(input_file);

   uint32_t sequence_count = 0;

   const auto column_names = metadata_reader.reader.get_col_names();
   for (auto& row : metadata_reader.reader) {
      for (const auto& item : database_config.schema.metadata) {
         const std::string value = row[item.name].get();
         const auto column_type = item.getColumnType();
         if (column_type == silo::config::ColumnType::INDEXED_STRING) {
            indexed_string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::STRING) {
            string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::INDEXED_PANGOLINEAGE) {
            pango_lineage_columns.at(item.name).insert({value});
         } else if (column_type == silo::config::ColumnType::DATE) {
            date_columns.at(item.name).insert(common::stringToDate(value));
         } else if (column_type == silo::config::ColumnType::INT) {
            int_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::FLOAT) {
            float_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::NUC_INSERTION) {
            nuc_insertion_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::AA_INSERTION) {
            aa_insertion_columns.at(item.name).insert(value);
         }
      }
      if (++sequence_count == UINT32_MAX) {
         throw std::runtime_error(
            "SILO is currently limited to UINT32_MAX=" + std::to_string(UINT32_MAX) + " sequences."
         );
      }
   }

   return sequence_count;
}

template <>
const std::map<std::string, storage::column::InsertionColumnPartition<NUCLEOTIDE_SYMBOL>&>&
ColumnPartitionGroup::getInsertionColumns<NUCLEOTIDE_SYMBOL>() const {
   return this->nuc_insertion_columns;
}

template <>
const std::map<std::string, storage::column::InsertionColumnPartition<AA_SYMBOL>&>&
ColumnPartitionGroup::getInsertionColumns<AA_SYMBOL>() const {
   return this->aa_insertion_columns;
}

ColumnPartitionGroup ColumnPartitionGroup::getSubgroup(
   const std::vector<silo::storage::ColumnMetadata>& fields
) const {
   ColumnPartitionGroup result;
   for (auto& field : fields) {
      result.metadata.push_back({field.name, field.type});
   }

   for (const auto& item : fields) {
      if (item.type == silo::config::ColumnType::INDEXED_STRING) {
         result.indexed_string_columns.insert({item.name, indexed_string_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::STRING) {
         result.string_columns.insert({item.name, string_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::INDEXED_PANGOLINEAGE) {
         result.pango_lineage_columns.insert({item.name, pango_lineage_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::DATE) {
         result.date_columns.insert({item.name, date_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::INT) {
         result.int_columns.insert({item.name, int_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::FLOAT) {
         result.float_columns.insert({item.name, float_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::NUC_INSERTION) {
         result.nuc_insertion_columns.insert({item.name, nuc_insertion_columns.at(item.name)});
      } else if (item.type == silo::config::ColumnType::AA_INSERTION) {
         result.aa_insertion_columns.insert({item.name, aa_insertion_columns.at(item.name)});
      }
   }
   return result;
}

std::optional<std::variant<std::string, int32_t, double>> ColumnPartitionGroup::getValue(
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
         .lookupValue(pango_lineage_columns.at(column).getValues().at(sequence_id))
         .value;
   }
   if (date_columns.contains(column)) {
      return common::dateToString(date_columns.at(column).getValues().at(sequence_id));
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

}  // namespace silo::storage
