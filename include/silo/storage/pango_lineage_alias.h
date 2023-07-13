#ifndef SILO_PANGO_LINEAGE_ALIAS_H
#define SILO_PANGO_LINEAGE_ALIAS_H

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace silo {

namespace common {
struct UnaliasedPangoLineage;
struct RawPangoLineage;

}  // namespace common

class PangoLineageAliasLookup {
  private:
   std::unordered_map<std::string, std::vector<std::string>> alias_key;

  public:
   PangoLineageAliasLookup() = default;

   explicit PangoLineageAliasLookup(
      std::unordered_map<std::string, std::vector<std::string>> alias_key
   );

   [[nodiscard]] common::UnaliasedPangoLineage unaliasPangoLineage(
      const common::RawPangoLineage& pango_lineage
   ) const;

   static silo::PangoLineageAliasLookup readFromFile(
      const std::filesystem::path& pango_lineage_alias_file
   );
};

}  // namespace silo

#endif  // SILO_PANGO_LINEAGE_ALIAS_H
