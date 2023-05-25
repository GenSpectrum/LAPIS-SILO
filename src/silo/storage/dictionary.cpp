#include "silo/storage/dictionary.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "silo/database.h"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo {

template <typename T>
TypedColumnsValueLookup<T>::TypedColumnsValueLookup(
   std::unordered_map<ColumnName, std::unordered_map<T, ValueId>> value_id_lookup,
   std::unordered_map<ColumnName, std::vector<T>> value_lookup
)
    : value_id_lookup(value_id_lookup),
      value_lookup(value_lookup) {}

template <typename T>
TypedColumnsValueLookup<T> TypedColumnsValueLookup<T>::createFromColumnNames(
   const std::vector<ColumnName>& column_names
) {
   std::unordered_map<ColumnName, std::unordered_map<T, ValueId>> value_id_lookup;
   std::unordered_map<ColumnName, std::vector<T>> value_lookup;

   for (const auto& column_name : column_names) {
      value_id_lookup[column_name] = std::unordered_map<std::string, ValueId>();
      value_lookup[column_name] = std::vector<std::string>();
   }

   return TypedColumnsValueLookup(value_id_lookup, value_lookup);
}

template <typename T>
void TypedColumnsValueLookup<T>::insertValue(const ColumnName& column_name, const T& value) {
   if (!value_id_lookup[column_name].contains(value)) {
      value_lookup.at(column_name).push_back(value);
      value_id_lookup.at(column_name)[value] = value_id_lookup.at(column_name).size();
   }
}

template <typename T>
std::optional<ValueId> TypedColumnsValueLookup<T>::lookupValueId(
   const silo::ColumnName& column_name,
   const T& value
) const {
   if (value_id_lookup.at(column_name).contains(value)) {
      return value_id_lookup.at(column_name).at(value);
   }
   return std::nullopt;
}

template <typename T>
std::optional<std::string> TypedColumnsValueLookup<T>::lookupValue(
   const silo::ColumnName& column_name,
   silo::ValueId value_id
) const {
   if (value_lookup.at(column_name).size() > value_id) {
      return value_lookup.at(column_name).at(value_id);
   }
   return std::nullopt;
}

Dictionary::Dictionary(
   const std::vector<ColumnName>& string_column_names,
   const std::vector<ColumnName>& pango_lineage_column_names
)
    : string_columns_lookup(
         TypedColumnsValueLookup<std::string>::createFromColumnNames(string_column_names)
      ),
      pango_lineage_columns_lookup(
         TypedColumnsValueLookup<PangoLineage>::createFromColumnNames(pango_lineage_column_names)
      ) {}

Dictionary::Dictionary()
    : Dictionary(
         std::vector<std::string>({"country", "region", "division"}),
         std::vector<std::string>({"pango_lineage"})
      ) {}

void Dictionary::updateDictionary(
   const std::filesystem::path& metadata_file,
   const silo::PangoLineageAliasLookup& alias_key
) {
   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(metadata_file);

   const std::vector<std::string> known_headers{
      "gisaid_epi_isl",
      "pango_lineage",
      "date",
      "region",
      "country",
      "division",
   };

   const std::vector<std::string>& vector = metadata_reader.get_col_names();

   // TODO(#82) check whether this is necessary
   for (const auto& header : known_headers) {
      if (std::find(vector.begin(), vector.end(), header) == vector.end()) {
         throw silo::PreprocessingException(
            "Metadata file does not contain field '" + header + "'"
         );
      }
   }

   // TODO(#82) when doing this, it should be a lot easier to bring back this piece of code -
   //          if it's necessary at all.
   //      while (!header_in.eof()) {
   //         getline(header_in, col_name, '\t');
   //         additional_columns_lookup.push_back(col_name);
   //         additional_columns_dictionary[col_name] = additional_columns_count++;
   //      }

   for (auto& row : metadata_reader) {
      const std::string pango_lineage_raw = alias_key.resolvePangoLineageAlias(
         row[silo::preprocessing::COLUMN_NAME_PANGO_LINEAGE].get()
      );
      const std::string region = row["region"].get();
      const std::string country = row["country"].get();
      const std::string division = row["division"].get();

      const auto pango_lineage = alias_key.resolvePangoLineageAlias(pango_lineage_raw);

      pango_lineage_columns_lookup.insertValue("pango_lineage", pango_lineage);
      string_columns_lookup.insertValue("region", region);
      string_columns_lookup.insertValue("country", country);
      string_columns_lookup.insertValue("division", division);
   }
}

void Dictionary::saveDictionary(std::ostream& dictionary_file) const {
   ::boost::archive::binary_oarchive output_archive(dictionary_file);
   output_archive << *this;
}

Dictionary Dictionary::loadDictionary(std::istream& dictionary_file) {
   Dictionary dictionary;

   ::boost::archive::binary_iarchive input_archive(dictionary_file);
   input_archive >> dictionary;

   return dictionary;
}

std::optional<ValueId> Dictionary::getPangoLineageIdInLookup(const std::string& pango_lineage
) const {
   return pango_lineage_columns_lookup.lookupValueId("pango_lineage", pango_lineage);
}

std::string Dictionary::getPangoLineage(uint32_t pango_lineage_id_in_lookup) const {
   return pango_lineage_columns_lookup.value_lookup.at("pango_lineage")
      .at(pango_lineage_id_in_lookup);
}

std::optional<ValueId> Dictionary::getCountryIdInLookup(const std::string& country) const {
   return lookupValueId("country", country);
}

std::string Dictionary::getCountry(uint32_t country_id_in_lookup) const {
   return lookupStringValue("country", country_id_in_lookup).value_or("");
}

std::optional<ValueId> Dictionary::getRegionIdInLookup(const std::string& region) const {
   return lookupValueId("region", region);
}

std::string Dictionary::getRegion(uint32_t region_lookup_id) const {
   return lookupStringValue("region", region_lookup_id).value_or("");
}

uint32_t Dictionary::getPangoLineageCount() const {
   return pango_lineage_columns_lookup.value_lookup.at("pango_lineage").size();
}
uint32_t Dictionary::getCountryCount() const {
   return string_columns_lookup.value_lookup.at("country").size();
}
uint32_t Dictionary::getRegionCount() const {
   return string_columns_lookup.value_lookup.at("region").size();
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<uint64_t> Dictionary::getIdInGeneralLookup(const std::string& value) const {
   return UINT32_MAX;
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Dictionary::getGeneralLookup(uint64_t /*general_id_in_lookup*/) const {
   return "";
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<uint32_t> Dictionary::getColumnIdInLookup(const std::string& /*column_name*/) const {
   return UINT32_MAX;
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Dictionary::getColumn(uint32_t /*column_id_in_lookup*/) const {
   return "";
}

std::optional<ValueId> Dictionary::lookupValueId(
   const ColumnName& column_name,
   const std::string& value
) const {
   return string_columns_lookup.lookupValueId(column_name, value);
};

std::optional<std::string> Dictionary::lookupStringValue(
   const ColumnName& column_name,
   ValueId value_id
) const {
   return string_columns_lookup.lookupValue(column_name, value_id);
}

std::optional<std::string> Dictionary::lookupPangoLineageValue(
   const ColumnName& column_name,
   ValueId value_id
) const {
   return pango_lineage_columns_lookup.lookupValue(column_name, value_id);
}

}  // namespace silo