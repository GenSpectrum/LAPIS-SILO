#include "silo/storage/metadata_store.h"

#include "silo/common/date.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

unsigned MetadataStore::fill(
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
         } else if (column_type == silo::config::ColumnType::FLOAT) {
            float_columns.at(item.name).insert(std::stod(value));
         }
      }
      ++sequence_count;
   }

   return sequence_count;
}

}  // namespace silo
