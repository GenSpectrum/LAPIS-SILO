#include "silo/storage/metadata_store.h"

#include "silo/common/time.h"
#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

unsigned MetadataStore::fill(
   const std::filesystem::path& input_file,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config
) {
   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(input_file);

   initializeColumns(database_config);

   unsigned sequence_count = 0;

   const auto column_names = metadata_reader.get_col_names();
   for (auto& row : metadata_reader) {
      for (const auto& item : database_config.schema.metadata) {
         const std::string value = row[item.name].get();

         const auto column_type = item.getColumnType();
         if (column_type == silo::config::ColumnType::INDEXED_STRING) {
            indexed_string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::STRING) {
            raw_string_columns.at(item.name).insert(value);
         } else if (column_type == silo::config::ColumnType::INDEXED_PANGOLINEAGE) {
            const std::string pango_lineage = alias_key.resolvePangoLineageAlias(value);
            pango_lineage_columns.at(item.name).insert({pango_lineage});
         } else if (column_type == silo::config::ColumnType::DATE) {
            date_columns.at(item.name).insert(common::mapToTime(value));
         }
      }
      ++sequence_count;
   }

   return sequence_count;
}

void MetadataStore::initializeColumns(const config::DatabaseConfig& database_config) {
   for (const auto& item : database_config.schema.metadata) {
      const auto column_type = item.getColumnType();

      if (column_type == config::ColumnType::INDEXED_STRING) {
         this->indexed_string_columns.emplace(item.name, storage::column::IndexedStringColumn());
      } else if (column_type == config::ColumnType::STRING) {
         this->raw_string_columns.emplace(item.name, storage::column::RawStringColumn());
      } else if (column_type == config::ColumnType::INDEXED_PANGOLINEAGE) {
         pango_lineage_columns.emplace(item.name, storage::column::PangoLineageColumn());
      } else if (column_type == config::ColumnType::DATE) {
         const auto column = item.name == database_config.schema.date_to_sort_by
                                ? storage::column::DateColumn(true)
                                : storage::column::DateColumn(false);
         date_columns.emplace(item.name, column);
      }
   }
}

}  // namespace silo
