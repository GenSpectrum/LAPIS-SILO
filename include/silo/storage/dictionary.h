#ifndef SILO_DICTIONARY_H
#define SILO_DICTIONARY_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace silo {
class PangoLineageAliasLookup;

class Dictionary {
  private:
   // TODO(someone) put this into two way map
   std::unordered_map<std::string, uint32_t> pango_lineage_dictionary;
   std::unordered_map<std::string, uint32_t> country_dictionary;
   std::unordered_map<std::string, uint32_t> region_dictionary;
   std::unordered_map<std::string, uint32_t> additional_columns_dictionary;
   std::unordered_map<std::string, uint64_t> general_dictionary;  // TODO(someone) clean this up!

   std::vector<std::string> pango_lineage_lookup;
   std::vector<std::string> country_lookup;
   std::vector<std::string> region_lookup;
   std::vector<std::string> additional_columns_lookup;
   std::vector<std::string> general_lookup;

   uint32_t pango_lineage_count = 0;
   uint32_t country_count = 0;
   uint32_t region_count = 0;
   uint32_t additional_columns_count = 0;
   uint64_t general_count = 0;

  public:
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
};

};  // namespace silo

#endif  // SILO_DICTIONARY_H
