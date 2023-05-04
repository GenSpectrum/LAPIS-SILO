#include "silo/storage/metadata_store.h"

#include <ctime>
#include <vector>

#include "silo/preprocessing/metadata.h"
#include "silo/storage/dictionary.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

unsigned MetadataStore::fill(
   const std::filesystem::path& input_file,
   const PangoLineageAliasLookup& alias_key,
   const Dictionary& dict
) {
   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(input_file);

   unsigned sequence_count = 0;

   for (auto& row : metadata_reader) {
      const std::string key = row[silo::preprocessing::COLUMN_NAME_PRIMARY_KEY].get();
      const std::string pango_lineage = alias_key.resolvePangoLineageAlias(
         row[silo::preprocessing::COLUMN_NAME_PANGO_LINEAGE].get()
      );
      const std::string date = row[silo::preprocessing::COLUMN_NAME_DATE].get();
      const std::string region = row["region"].get();
      const std::string country = row["country"].get();
      const std::string division = row["division"].get();

      struct std::tm time_struct {};
      std::istringstream time_stream(date);
      time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
      std::time_t const time = mktime(&time_struct);

      std::vector<uint64_t> extra_cols;
      extra_cols.push_back(dict.getIdInGeneralLookup(division));

      inputSequenceMeta(
         key,
         time,
         dict.getPangoLineageIdInLookup(pango_lineage),
         dict.getRegionIdInLookup(region),
         dict.getCountryIdInLookup(country),
         extra_cols
      );
      ++sequence_count;
   }

   return sequence_count;
}

// TODO(someone): clean up and specify inputs
void MetadataStore::inputSequenceMeta(
   const std::string& primary_key,
   time_t date,
   uint32_t pango_lineage,
   uint32_t region,
   uint32_t country,
   const std::vector<uint64_t>& values
) {
   sequence_id_to_key.push_back(primary_key);
   sequence_id_to_lineage.push_back(pango_lineage);

   sequence_id_to_date.push_back(date);
   sequence_id_to_country.push_back(country);
   sequence_id_to_region.push_back(region);
   for (unsigned i = 0; i < columns.size(); ++i) {
      columns[i].push_back(values[i]);
   }
}

}  // namespace silo
