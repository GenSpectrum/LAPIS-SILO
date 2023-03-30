#include "silo/storage/pango_lineage_alias.h"

#include <iterator>
#include <sstream>

namespace silo {

PangoLineageAliasLookup::PangoLineageAliasLookup(
   std::unordered_map<std::string, std::string> alias_key
)
    : alias_key(std::move(alias_key)) {}

PangoLineageAliasLookup::PangoLineageAliasLookup() = default;

std::string PangoLineageAliasLookup::resolvePangoLineageAlias(const std::string& pango_lineage
) const {
   std::string pango_lineage_prefix;
   std::stringstream pango_lineage_stream(pango_lineage);
   getline(pango_lineage_stream, pango_lineage_prefix, '.');
   if (alias_key.contains(pango_lineage_prefix)) {
      if (pango_lineage_stream.eof()) {
         return alias_key.at(pango_lineage_prefix);
      }
      const std::string suffix(
         (std::istream_iterator<char>(pango_lineage_stream)), std::istream_iterator<char>()
      );
      return alias_key.at(pango_lineage_prefix) + '.' + suffix;
   }
   return pango_lineage;
}

}  // namespace silo
