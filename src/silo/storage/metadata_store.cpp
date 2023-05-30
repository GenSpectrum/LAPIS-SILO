#include "silo/storage/metadata_store.h"

#include <ctime>

#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/dictionary.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

unsigned MetadataStore::fill(
   const std::filesystem::path& input_file,
   const PangoLineageAliasLookup& alias_key,
   const Dictionary& dict,
   const silo::config::DatabaseConfig& database_config
) {
   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(input_file);

   const auto columns_to_index = std::set<std::string>{"country", "region", "division"};
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
            // TODO extend for pango lineages
            const std::string pango_lineage = alias_key.resolvePangoLineageAlias(value);
            sequence_id_to_lineage.push_back(
               dict.getPangoLineageIdInLookup(pango_lineage).value_or(0)
            );
         } else if (item.type == silo::config::DatabaseMetadataType::DATE) {
            // TODO extend for dates
            struct std::tm time_struct {};
            std::istringstream time_stream(value);
            time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
            std::time_t const time = mktime(&time_struct);
            sequence_id_to_date.push_back(time);
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
         // TODO extend for pango lineages
      } else if (item.type == config::DatabaseMetadataType::DATE) {
         // TODO extend for dates
      }
   }
}

}  // namespace silo
