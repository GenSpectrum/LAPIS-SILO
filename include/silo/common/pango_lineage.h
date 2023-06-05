#ifndef SILO_PANGO_LINEAGE_H
#define SILO_PANGO_LINEAGE_H

#include <string>

namespace silo::common {

struct PangoLineage {
   template <class Archive>
   void serialize(Archive& archive, const unsigned int /* version*/) {
      archive& value;
   }

   std::string value;

   bool isSublineageOf(const PangoLineage& other) const;

   bool operator<(const PangoLineage& other) const;
   bool operator==(const PangoLineage& other) const;
};

}  // namespace silo::common

template <>
struct std::hash<silo::common::PangoLineage> {
   std::size_t operator()(const silo::common::PangoLineage& pango_lineage) const;
};

#endif  // SILO_PANGO_LINEAGE_H
