#include "silo/preprocessing/pango_lineage_count.h"

#include <algorithm>
#include <climits>
#include <iostream>
#include <unordered_map>

#include "silo/storage/pango_lineage_alias.h"

namespace silo::preprocessing {

void PangoLineageCounts::save(std::ostream& output_file) {
   for (const auto& pango_lineage_count : pango_lineage_counts) {
      output_file << pango_lineage_count.pango_lineage << '\t'
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
      descriptor.pango_lineage_counts.emplace_back(PangoLineageCount{lineage, count});
   }
   return descriptor;
}

PangoLineageCounts buildPangoLineageCounts(
   const PangoLineageAliasLookup& alias_key,
   std::istream& meta_in
) {
   PangoLineageCounts pango_lineage_counts;
   // Ignore header line.
   meta_in.ignore(LONG_MAX, '\n');

   uint32_t pid_count = 0;

   std::unordered_map<std::string, uint32_t> pango_to_id;

   while (true) {
      std::string epi_isl;
      std::string pango_lineage_raw;
      if (!getline(meta_in, epi_isl, '\t')) {
         break;
      }
      if (!getline(meta_in, pango_lineage_raw, '\t')) {
         break;
      }
      meta_in.ignore(LONG_MAX, '\n');

      /// Deal with pango_lineage alias:
      std::string const pango_lineage = alias_key.resolvePangoLineageAlias(pango_lineage_raw);

      if (pango_to_id.contains(pango_lineage)) {
         auto pid = pango_to_id[pango_lineage];
         ++pango_lineage_counts.pango_lineage_counts[pid].count_of_sequences;
      } else {
         pango_to_id[pango_lineage] = pid_count++;
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
