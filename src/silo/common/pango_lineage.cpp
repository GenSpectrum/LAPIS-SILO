#include "silo/common/pango_lineage.h"

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

bool PangoLineage::operator<(const PangoLineage& other) const {
   return value < other.value;
}

bool PangoLineage::operator==(const PangoLineage& other) const {
   return value == other.value;
}

}  // namespace silo::common
