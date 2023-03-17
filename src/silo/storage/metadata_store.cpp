#include <silo/storage/metadata_store.h>

// TODO(someone): clean up and specify inputs
void silo::inputSequenceMeta(
   MetadataStore& metadata_store,
   uint64_t epi_isl_number,  // NOLINT
   time_t date,
   uint32_t pango_lineage,
   uint32_t region,
   uint32_t country,
   const std::vector<uint64_t>& values
) {
   metadata_store.sequence_id_to_epi.push_back(epi_isl_number);
   metadata_store.sequence_id_to_lineage.push_back(pango_lineage);

   metadata_store.sequence_id_to_date.push_back(date);
   metadata_store.sequence_id_to_country.push_back(country);
   metadata_store.sequence_id_to_region.push_back(region);
   for (unsigned i = 0; i < metadata_store.columns.size(); ++i) {
      metadata_store.columns[i].push_back(values[i]);
   }
}
