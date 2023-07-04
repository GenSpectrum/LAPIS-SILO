#include "silo/storage/column_group.h"

#include "silo/common/date.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo::storage {

unsigned ColumnGroup::fill(
   const std::filesystem::path& input_file,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config
) {
   auto metadata_reader = silo::preprocessing::MetadataReader(input_file);

   unsigned sequence_count = 0;

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
            const std::string pango_lineage = alias_key.resolvePangoLineageAlias(value);
            pango_lineage_columns.at(item.name).insert({pango_lineage});
         } else if (column_type == silo::config::ColumnType::DATE) {
            date_columns.at(item.name).insert(common::stringToDate(value));
         } else if (column_type == silo::config::ColumnType::INT) {
            const int32_t int_value = value.empty() ? INT32_MIN : std::stoi(value);
            int_columns.at(item.name).insert(int_value);
         } else if (column_type == silo::config::ColumnType::FLOAT) {
            const double double_value = value.empty() ? std::nan("") : std::stod(value);
            float_columns.at(item.name).insert(double_value);
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
      } else if (column_type == silo::config::ColumnType::FLOAT) {
         result.float_columns.insert({item.name, float_columns.at(item.name)});
      }
   }
   return result;
}

}  // namespace silo::storage
