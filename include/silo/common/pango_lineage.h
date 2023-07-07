#ifndef SILO_PANGO_LINEAGE_H
#define SILO_PANGO_LINEAGE_H

#include <string>
#include <vector>

namespace silo::common {

struct PangoLineage {
   std::string value;

   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version*/) {
      // clang-format off
      archive& value;
      // clang-format on
   }

   bool isSublineageOf(const PangoLineage& other) const;

   std::vector<PangoLineage> getParentLineages() const;

   bool operator<(const PangoLineage& other) const;
   bool operator==(const PangoLineage& other) const;
};

}  // namespace silo::common

template <>
struct std::hash<silo::common::PangoLineage> {
   std::size_t operator()(const silo::common::PangoLineage& pango_lineage) const;
};

#endif  // SILO_PANGO_LINEAGE_H
