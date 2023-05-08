#ifndef SILO_DICTIONARY_H
#define SILO_DICTIONARY_H

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace silo {
class PangoLineageAliasLookup;

using ColumnName = std::string;
using ValueId = uint32_t;

class Dictionary {
  private:
   std::unordered_map<ColumnName, std::unordered_map<std::string, ValueId>> stringLookup;
   std::unordered_map<ColumnName, std::vector<std::string>> reverseStringLookup;

   void updateStringLookup(const std::string& column_name, const std::string& value);
   std::unordered_map<std::string, uint32_t> pango_lineage_dictionary;

   std::vector<std::string> pango_lineage_lookup;
   uint32_t pango_lineage_count = 0;

  public:
   explicit Dictionary();
   explicit Dictionary(const std::vector<std::string>& column_names);

   void updateDictionary(
      const std::filesystem::path& metadata_file,
      const silo::PangoLineageAliasLookup& alias_key
   );

   void saveDictionary(std::ostream& dictionary_file) const;

   static Dictionary loadDictionary(std::istream& dictionary_file);

   [[maybe_unused]] [[nodiscard]] uint32_t getPangoLineageIdInLookup(
      const std::string& pango_lineage
   ) const;

   [[maybe_unused]] [[nodiscard]] const std::string& getPangoLineage(
      uint32_t pango_lineage_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getPangoLineageCount() const;

   [[maybe_unused]] [[nodiscard]] uint32_t getCountryIdInLookup(const std::string& country) const;

   [[maybe_unused]] [[nodiscard]] const std::string& getCountry(uint32_t country_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getCountryCount() const;

   [[maybe_unused]] [[nodiscard]] uint32_t getRegionIdInLookup(const std::string& region) const;

   [[maybe_unused]] [[nodiscard]] const std::string& getRegion(uint32_t region_lookup_id) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getRegionCount() const;

   [[maybe_unused]] [[nodiscard]] uint64_t getIdInGeneralLookup(
      const std::string& region_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] const std::string& getGeneralLookup(uint64_t general_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getColumnIdInLookup(const std::string& column_name
   ) const;

   [[maybe_unused]] [[nodiscard]] const std::string& getColumn(uint32_t column_id_in_lookup) const;

   [[maybe_unused]] std::optional<ValueId> lookupValueId(
      const std::string& column_name,
      const std::string& value
   ) const;

   std::optional<std::string> lookupStringValue(const std::string& column_name, ValueId value_id)
      const;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive & stringLookup;
      archive & reverseStringLookup;
      archive & pango_lineage_dictionary;
      archive & pango_lineage_lookup;
      archive & pango_lineage_count;
   }
};

};  // namespace silo

#endif  // SILO_DICTIONARY_H
