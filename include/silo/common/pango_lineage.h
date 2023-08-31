#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace silo::common {

struct RawPangoLineage {
   std::string value;
};

struct UnaliasedPangoLineage {
   std::string value;

   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version*/) {
      // clang-format off
      archive & value;
      // clang-format on
   }

   bool isSublineageOf(const UnaliasedPangoLineage& other) const;

   std::vector<UnaliasedPangoLineage> getParentLineages() const;

   bool operator<(const UnaliasedPangoLineage& other) const;
   bool operator==(const UnaliasedPangoLineage& other) const;
};

}  // namespace silo::common

template <>
struct std::hash<silo::common::UnaliasedPangoLineage> {
   std::size_t operator()(const silo::common::UnaliasedPangoLineage& pango_lineage) const;
};
