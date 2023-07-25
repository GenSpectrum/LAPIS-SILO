#include "silo/common/pango_lineage.h"

#include <compare>

std::size_t std::hash<silo::common::UnaliasedPangoLineage>::operator()(
   const silo::common::UnaliasedPangoLineage& pango_lineage
) const {
   return hash<string>()(pango_lineage.value);
}

namespace silo::common {

bool UnaliasedPangoLineage::isSublineageOf(const silo::common::UnaliasedPangoLineage& other) const {
   if (other.value.length() > value.length()) {
      return false;
   }

   return value.starts_with(other.value);
}

std::vector<UnaliasedPangoLineage> UnaliasedPangoLineage::getParentLineages() const {
   std::vector<UnaliasedPangoLineage> parent_lineages;

   std::string::size_type pos = 0;
   while (pos != std::string::npos) {
      pos = value.find('.', pos + 1);
      parent_lineages.push_back(UnaliasedPangoLineage{value.substr(0, pos)});
   }

   return parent_lineages;
}

bool UnaliasedPangoLineage::operator<(const UnaliasedPangoLineage& other) const {
   return value < other.value;
}

bool UnaliasedPangoLineage::operator==(const UnaliasedPangoLineage& other) const {
   return value == other.value;
}

}  // namespace silo::common
