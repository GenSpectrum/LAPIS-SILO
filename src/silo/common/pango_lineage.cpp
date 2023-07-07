#include "silo/common/pango_lineage.h"

#include <compare>

std::size_t std::hash<silo::common::PangoLineage>::operator()(
   const silo::common::PangoLineage& pango_lineage
) const {
   return hash<string>()(pango_lineage.value);
}

namespace silo::common {

bool PangoLineage::isSublineageOf(const silo::common::PangoLineage& other) const {
   if (other.value.length() > value.length()) {
      return false;
   }

   return value.starts_with(other.value);
}

std::vector<PangoLineage> PangoLineage::getParentLineages() const {
   std::vector<PangoLineage> parent_lineages;

   std::string::size_type pos = 0;
   while (pos != std::string::npos) {
      pos = value.find('.', pos + 1);
      parent_lineages.push_back(PangoLineage{value.substr(0, pos)});
   }

   return parent_lineages;
}

bool PangoLineage::operator<(const PangoLineage& other) const {
   return value < other.value;
}

bool PangoLineage::operator==(const PangoLineage& other) const {
   return value == other.value;
}

}  // namespace silo::common
