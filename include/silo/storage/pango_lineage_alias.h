#ifndef SILO_PANGO_LINEAGE_ALIAS_H
#define SILO_PANGO_LINEAGE_ALIAS_H

#include <filesystem>
#include <string>
#include <unordered_map>

namespace silo {

class PangoLineageAliasLookup {
  private:
   std::unordered_map<std::string, std::string> alias_key;

  public:
   PangoLineageAliasLookup() = default;
   explicit PangoLineageAliasLookup(std::unordered_map<std::string, std::string> alias_key);

   std::string resolvePangoLineageAlias(const std::string& pango_lineage) const;

   static silo::PangoLineageAliasLookup readFromFile(
      const std::filesystem::path& pango_lineage_alias_file
   );
};

}  // namespace silo

#endif  // SILO_PANGO_LINEAGE_ALIAS_H
