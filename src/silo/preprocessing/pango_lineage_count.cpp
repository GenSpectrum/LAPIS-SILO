#include "silo/preprocessing/pango_lineage_count.h"

#include <silo/common/pango_lineage.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "silo/config/database_config.h"
#include "silo/preprocessing/metadata.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo::preprocessing {

void PangoLineageCounts::save(std::ostream& output_file) {
   for (const auto& pango_lineage_count : pango_lineage_counts) {
      output_file << pango_lineage_count.pango_lineage.value << '\t'
                  << pango_lineage_count.count_of_sequences << '\n';
   }
   output_file.flush();
}

PangoLineageCounts PangoLineageCounts::load(std::istream& input_stream) {
   PangoLineageCounts descriptor;
   std::string lineage;
   std::string count_str;
   uint32_t count;
   while (input_stream && !input_stream.eof()) {
      if (!getline(input_stream, lineage, '\t')) {
         break;
      }
      if (!getline(input_stream, count_str, '\n')) {
         break;
      }
      count = atoi(count_str.c_str());
      descriptor.pango_lineage_counts.emplace_back(PangoLineageCount{{lineage}, count});
   }
   return descriptor;
}

PangoLineageCounts buildPangoLineageCounts(
   const PangoLineageAliasLookup& alias_key,
   const std::filesystem::path& metadata_path,
   const silo::config::DatabaseConfig& database_config
) {
   PangoLineageCounts pango_lineage_counts;

   uint32_t pango_lineage_ids_count = 0;
   std::unordered_map<common::UnaliasedPangoLineage, uint32_t> pango_lineage_to_id;

   const std::vector<std::string> unresolved_pango_lineages =
      silo::preprocessing::MetadataReader(metadata_path)
         .getColumn(database_config.schema.partition_by);

   for (const auto& unresolved_pango_lineage : unresolved_pango_lineages) {
      const common::UnaliasedPangoLineage pango_lineage =
         alias_key.unaliasPangoLineage({unresolved_pango_lineage});

      if (pango_lineage_to_id.contains(pango_lineage)) {
         auto pid = pango_lineage_to_id[pango_lineage];
         ++pango_lineage_counts.pango_lineage_counts[pid].count_of_sequences;
      } else {
         pango_lineage_to_id[pango_lineage] = pango_lineage_ids_count++;
         pango_lineage_counts.pango_lineage_counts.emplace_back(PangoLineageCount{pango_lineage, 1}
         );
      }
   }

   // Now sort alphabetically so that we get better compression.
   // -> similar PIDs next to each other in sequence_store -> better run-length compression
   std::sort(
      pango_lineage_counts.pango_lineage_counts.begin(),
      pango_lineage_counts.pango_lineage_counts.end(),
      [](const PangoLineageCount& lhs, const PangoLineageCount& rhs) {
         return lhs.pango_lineage < rhs.pango_lineage;
      }
   );
   return pango_lineage_counts;
}

}  // namespace silo::preprocessing
