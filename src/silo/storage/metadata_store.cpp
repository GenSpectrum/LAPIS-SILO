#include "silo/storage/metadata_store.h"

#include <ctime>
#include <vector>

#include "silo/storage/dictionary.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

unsigned MetadataStore::fill(
   std::istream& input_file,
   const PangoLineageAliasLookup& alias_key,
   const Dictionary& dict
) {
   // Ignore header line.
   input_file.ignore(LONG_MAX, '\n');

   unsigned sequence_count = 0;

   while (true) {
      std::string epi_isl;
      std::string pango_lineage_raw;
      std::string date;
      std::string region;
      std::string country;
      std::string division;
      if (!getline(input_file, epi_isl, '\t')) {
         break;
      }
      if (!getline(input_file, pango_lineage_raw, '\t')) {
         break;
      }
      if (!getline(input_file, date, '\t')) {
         break;
      }
      if (!getline(input_file, region, '\t')) {
         break;
      }
      if (!getline(input_file, country, '\t')) {
         break;
      }
      if (!getline(input_file, division, '\n')) {
         break;
      }

      std::string const pango_lineage = alias_key.resolvePangoLineageAlias(pango_lineage_raw);

      constexpr int START_POSITION_OF_NUMBER_IN_EPI_ISL = 8;
      std::string const tmp = epi_isl.substr(START_POSITION_OF_NUMBER_IN_EPI_ISL);
      uint64_t const epi = stoi(tmp);

      struct std::tm time_struct {};
      std::istringstream time_stream(date);
      time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
      std::time_t const time = mktime(&time_struct);

      std::vector<uint64_t> extra_cols;
      extra_cols.push_back(dict.getIdInGeneralLookup(division));

      inputSequenceMeta(
         epi,
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
   uint64_t epi_isl_number,  // NOLINT
   time_t date,
   uint32_t pango_lineage,
   uint32_t region,
   uint32_t country,
   const std::vector<uint64_t>& values
) {
   sequence_id_to_epi.push_back(epi_isl_number);
   sequence_id_to_lineage.push_back(pango_lineage);

   sequence_id_to_date.push_back(date);
   sequence_id_to_country.push_back(country);
   sequence_id_to_region.push_back(region);
   for (unsigned i = 0; i < columns.size(); ++i) {
      columns[i].push_back(values[i]);
   }
}

}  // namespace silo
