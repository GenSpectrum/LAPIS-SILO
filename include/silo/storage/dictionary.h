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

template <typename T>
class TypedColumnsValueLookup {
  public:  // TODO(#101) make this private
   std::unordered_map<ColumnName, std::unordered_map<T, ValueId>> value_id_lookup;
   std::unordered_map<ColumnName, std::vector<T>> value_lookup;

  private:
   TypedColumnsValueLookup(
      std::unordered_map<ColumnName, std::unordered_map<T, ValueId>> value_id_lookup,
      std::unordered_map<ColumnName, std::vector<T>> value_lookup
   );

  public:
   static TypedColumnsValueLookup<T> createFromColumnNames(
      const std::vector<ColumnName>& column_names
   );

   void insertValue(const ColumnName& column_name, const T& value);

   std::optional<ValueId> lookupValueId(const ColumnName& column_name, const T& value) const;

   std::optional<std::string> lookupValue(const ColumnName& column_name, ValueId value_id) const;

   // Workaround until GH-Action jidicula/clang-format-action supports clang-format-17
   // clang-format off
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& value_id_lookup;
      archive& value_lookup;
   }
   // clang-format on
};

class Dictionary {
  private:
   TypedColumnsValueLookup<std::string> string_columns_lookup;
   TypedColumnsValueLookup<PangoLineage> pango_lineage_columns_lookup;

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

   virtual std::optional<ValueId> lookupValueId(
      const ColumnName& column_name,
      const std::string& value
   ) const;

   std::optional<std::string> lookupStringValue(const ColumnName& column_name, ValueId value_id)
      const;

   std::optional<PangoLineage> lookupPangoLineageValue(
      const ColumnName& column_name,
      ValueId value_id
   ) const;

   // Workaround until GH-Action jidicula/clang-format-action supports clang-format-17
   // clang-format off
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& string_columns_lookup;
      archive& pango_lineage_columns_lookup;
   }
   // clang-format on
};

};  // namespace silo

#endif  // SILO_DICTIONARY_H
