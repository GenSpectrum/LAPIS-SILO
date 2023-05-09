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
using PangoLineage = std::string;

class Dictionary {
  private:
   std::unordered_map<ColumnName, std::unordered_map<std::string, ValueId>> string_lookup;
   std::unordered_map<ColumnName, std::vector<std::string>> reverse_string_lookup;
   void updateStringLookup(const ColumnName& column_name, const std::string& value);

   std::unordered_map<ColumnName, std::unordered_map<PangoLineage, ValueId>> pango_lineage_lookup;
   std::unordered_map<ColumnName, std::vector<PangoLineage>> reverse_pango_lineage_lookup;
   void updatePangoLineageLookup(const ColumnName& column_name, const PangoLineage& pango_lineage);

  public:
   explicit Dictionary();
   explicit Dictionary(
      const std::vector<ColumnName>& string_column_names,
      const std::vector<ColumnName>& pango_lineage_column_names
   );

   void updateDictionary(
      const std::filesystem::path& metadata_file,
      const silo::PangoLineageAliasLookup& alias_key
   );

   void saveDictionary(std::ostream& dictionary_file) const;

   static Dictionary loadDictionary(std::istream& dictionary_file);

   [[maybe_unused]] [[nodiscard]] uint32_t getPangoLineageIdInLookup(
      const std::string& pango_lineage
   ) const;

   [[maybe_unused]] [[nodiscard]] std::string getPangoLineage(uint32_t pango_lineage_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getPangoLineageCount() const;

   [[maybe_unused]] [[nodiscard]] uint32_t getCountryIdInLookup(const std::string& country) const;

   [[maybe_unused]] [[nodiscard]] std::string getCountry(uint32_t country_id_in_lookup) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getCountryCount() const;

   [[maybe_unused]] [[nodiscard]] uint32_t getRegionIdInLookup(const std::string& region) const;

   [[maybe_unused]] [[nodiscard]] std::string getRegion(uint32_t region_lookup_id) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getRegionCount() const;

   [[maybe_unused]] [[nodiscard]] uint64_t getIdInGeneralLookup(
      const std::string& region_id_in_lookup
   ) const;

   [[maybe_unused]] [[nodiscard]] std::string getGeneralLookup(uint64_t general_id_in_lookup) const;

   [[maybe_unused]] [[nodiscard]] uint32_t getColumnIdInLookup(const std::string& column_name
   ) const;

   [[maybe_unused]] [[nodiscard]] std::string getColumn(uint32_t column_id_in_lookup) const;

   [[maybe_unused]] std::optional<ValueId> lookupValueId(
      const std::string& column_name,
      const std::string& value
   ) const;

   std::optional<std::string> lookupStringValue(const std::string& column_name, ValueId value_id)
      const;

   std::optional<PangoLineage> lookupPangoLineageValue(
      const std::string& column_name,
      ValueId value_id
   ) const;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive & string_lookup;
      archive & reverse_string_lookup;
      archive & pango_lineage_lookup;
      archive & reverse_pango_lineage_lookup;
   }
};

};  // namespace silo

#endif  // SILO_DICTIONARY_H
