#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {

namespace common {
struct AliasedPangoLineage;
struct UnaliasedPangoLineage;
struct RawPangoLineage;

}  // namespace common

class PangoLineageAliasLookup {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & alias_key;
      // clang-format on
   }

   std::unordered_map<std::string, std::vector<std::string>> alias_key;

  public:
   PangoLineageAliasLookup() = default;

   explicit PangoLineageAliasLookup(
      std::unordered_map<std::string, std::vector<std::string>> alias_key
   );

   [[nodiscard]] common::UnaliasedPangoLineage unaliasPangoLineage(
      const common::RawPangoLineage& pango_lineage
   ) const;

   [[nodiscard]] common::AliasedPangoLineage aliasPangoLineage(
      const common::UnaliasedPangoLineage& pango_lineage
   ) const;

   static silo::PangoLineageAliasLookup readFromFile(
      const std::filesystem::path& pango_lineage_alias_file
   );
};

}  // namespace silo
