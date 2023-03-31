#ifndef SILO_PANGO_LINEAGE_COUNT_H
#define SILO_PANGO_LINEAGE_COUNT_H

#include <string>
#include <vector>

namespace silo {
struct PangoLineageAliasLookup;

namespace preprocessing {

struct PangoLineageCount {
   std::string pango_lineage;
   uint32_t count_of_sequences;
};

struct PangoLineageCounts {
   std::vector<PangoLineageCount> pango_lineage_counts;

   void save(std::ostream& output_file);

   static PangoLineageCounts load(std::istream& input_stream);
};

PangoLineageCounts buildPangoLineageCounts(
   const PangoLineageAliasLookup& alias_key,
   std::istream& meta_in
);

}  // namespace preprocessing
}  // namespace silo
#endif  // SILO_PANGO_LINEAGE_COUNT_H
