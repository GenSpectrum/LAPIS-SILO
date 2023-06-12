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

   std::set<std::string> columns_to_index;
   for (const auto& element : database_config.schema.metadata) {
      if (element.generate_index) {
         columns_to_index.insert(element.name);
      }
   }

   initializeColumns(database_config, columns_to_index);

   unsigned sequence_count = 0;

   const auto column_names = metadata_reader.get_col_names();
   for (auto& row : metadata_reader) {
      for (const auto& item : database_config.schema.metadata) {
         const std::string value = row[item.name].get();

         if (item.type == silo::config::DatabaseMetadataType::STRING) {
            if (columns_to_index.contains(item.name)) {
               indexed_string_columns.at(item.name).insert(value);
            } else {
               raw_string_columns.at(item.name).insert(value);
            }
         } else if (item.type == silo::config::DatabaseMetadataType::PANGOLINEAGE) {
            const std::string pango_lineage = alias_key.resolvePangoLineageAlias(value);
            pango_lineage_columns.at(item.name).insert({pango_lineage});
         } else if (item.type == silo::config::DatabaseMetadataType::DATE) {
            date_columns.at(item.name).insert(common::mapToTime(value));
         }
      }
      ++sequence_count;
   }

   return sequence_count;
}

void MetadataStore::initializeColumns(
   const config::DatabaseConfig& database_config,
   const std::set<std::string>& columns_to_index
) {
   for (const auto& item : database_config.schema.metadata) {
      if (item.type == config::DatabaseMetadataType::STRING) {
         if (columns_to_index.contains(item.name)) {
            this->indexed_string_columns[item.name] = storage::column::IndexedStringColumn();
         } else {
            this->raw_string_columns[item.name] = storage::column::RawStringColumn();
         }
      } else if (item.type == config::DatabaseMetadataType::PANGOLINEAGE) {
         pango_lineage_columns.emplace(item.name, storage::column::PangoLineageColumn());
      } else if (item.type == config::DatabaseMetadataType::DATE) {
         const auto column =
            item.name == database_config.schema.date_to_sort_by
               ? storage::column::DateColumn(true)
               : storage::column::DateColumn(false);
         date_columns.emplace(item.name, column);
      }
   }
}

}  // namespace silo
