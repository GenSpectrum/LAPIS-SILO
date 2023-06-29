#include "silo/storage/column_group.h"

#include "silo/common/date.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo::storage {

const storage::column::DateColumnPartition& ColumnGroup::getDateColumn(const std::string& name
) const {
   return date_columns.at(name);
}

const storage::column::IndexedStringColumnPartition& ColumnGroup::getIndexedStringColumn(
   const std::string& name
) const {
   return indexed_string_columns.at(name);
}

const storage::column::StringColumnPartition& ColumnGroup::getStringColumn(const std::string& name
) const {
   return string_columns.at(name);
}

const storage::column::PangoLineageColumnPartition& ColumnGroup::getPangoLineageColumn(
   const std::string& name
) const {
   return pango_lineage_columns.at(name);
}

const storage::column::IntColumnPartition& ColumnGroup::getIntColumn(const std::string& name
) const {
   return int_columns.at(name);
}

unsigned ColumnGroup::fill(
   const std::filesystem::path& input_file,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config
) {
   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(input_file);

   unsigned sequence_count = 0;

   const auto column_names = metadata_reader.get_col_names();
   for (auto& row : metadata_reader) {
      for (const auto& item : database_config.schema.metadata) {
         const std::string value = row[item.name].get();

         const auto column_type = item.getColumnType();
         if (column_type == silo::config::ColumnType::INDEXED_STRING) {
            indexed_string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::STRING) {
            string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::INDEXED_PANGOLINEAGE) {
            const std::string pango_lineage = alias_key.resolvePangoLineageAlias(value);
            pango_lineage_columns.at(item.name).insert({pango_lineage});
         } else if (column_type == silo::config::ColumnType::DATE) {
            date_columns.at(item.name).insert(common::stringToDate(value));
         } else if (column_type == silo::config::ColumnType::INT) {
            int_columns.at(item.name).insert(stoi(value));
         }
      }
      ++sequence_count;
   }

   return sequence_count;
}

ColumnGroup ColumnGroup::getSubgroup(const std::vector<config::DatabaseMetadata>& fields) const {
   ColumnGroup result;
   result.metadata = fields;

   for (const auto& item : fields) {
      const auto column_type = item.getColumnType();
      if (column_type == silo::config::ColumnType::INDEXED_STRING) {
         result.indexed_string_columns.insert({item.name, indexed_string_columns.at(item.name)});
      } else if (column_type == silo::config::ColumnType::STRING) {
         result.string_columns.insert({item.name, string_columns.at(item.name)});
      } else if (column_type == silo::config::ColumnType::INDEXED_PANGOLINEAGE) {
         result.pango_lineage_columns.insert({item.name, pango_lineage_columns.at(item.name)});
      } else if (column_type == silo::config::ColumnType::DATE) {
         result.date_columns.insert({item.name, date_columns.at(item.name)});
      } else if (column_type == silo::config::ColumnType::INT) {
         result.int_columns.insert({item.name, int_columns.at(item.name)});
      }
   }
   return result;
}

}  // namespace silo::storage
