#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

#include "silo/common/pango_lineage.h"

namespace silo {
struct PangoLineageAliasLookup;

namespace config {
struct DatabaseConfig;
}  // namespace config

namespace preprocessing {

struct PangoLineageCount {
   common::UnaliasedPangoLineage pango_lineage;
   uint32_t count_of_sequences;
};

struct PangoLineageCounts {
   std::vector<PangoLineageCount> pango_lineage_counts;

   void save(std::ostream& output_file);

   static PangoLineageCounts load(std::istream& input_stream);
};

PangoLineageCounts buildPangoLineageCounts(
   const PangoLineageAliasLookup& alias_key,
   const std::filesystem::path& metadata_path,
   const std::string& partition_by
);

}  // namespace preprocessing
}  // namespace silo
