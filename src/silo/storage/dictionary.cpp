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

void Dictionary::updateStringLookup(const ColumnName& column_name, const std::string& value) {
   if (!string_lookup[column_name].contains(value)) {
      reverse_string_lookup.at(column_name).push_back(value);
      string_lookup.at(column_name)[value] = string_lookup.at(column_name).size();
   }
}

void Dictionary::updatePangoLineageLookup(
   const ColumnName& column_name,
   const PangoLineage& pango_lineage
) {
   if (!pango_lineage_lookup.at(column_name).contains(pango_lineage)) {
      reverse_pango_lineage_lookup.at(column_name).push_back(pango_lineage);
      pango_lineage_lookup.at(column_name)[pango_lineage] =
         pango_lineage_lookup.at(column_name).size();
   }
}

Dictionary::Dictionary(
   const std::vector<ColumnName>& string_column_names,
   const std::vector<ColumnName>& pango_lineage_column_names
) {
   for (const auto& column_name : string_column_names) {
      string_lookup[column_name] = std::unordered_map<std::string, ValueId>();
      reverse_string_lookup[column_name] = std::vector<std::string>();
   }

   for (const auto& column_name : pango_lineage_column_names) {
      pango_lineage_lookup[column_name] = std::unordered_map<PangoLineage, ValueId>();
      reverse_pango_lineage_lookup[column_name] = std::vector<PangoLineage>();
   }
}

Dictionary::Dictionary()
    : Dictionary(
         std::vector<std::string>({"country", "region", "division", "column"}),
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

      updatePangoLineageLookup("pango_lineage", pango_lineage);
      updateStringLookup("region", region);
      updateStringLookup("country", country);
      updateStringLookup("division", division);
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

uint32_t Dictionary::getPangoLineageIdInLookup(const std::string& pango_lineage) const {
   if (pango_lineage_lookup.at("pango_lineage").contains(pango_lineage)) {
      return pango_lineage_lookup.at("pango_lineage").at(pango_lineage);
   }
   return UINT32_MAX;
}

std::string error_string = "NOID";

std::string Dictionary::getPangoLineage(uint32_t pango_lineage_id_in_lookup) const {
   return reverse_pango_lineage_lookup.at("pango_lineage").at(pango_lineage_id_in_lookup);
}

uint32_t Dictionary::getCountryIdInLookup(const std::string& country) const {
   return lookupValueId("country", country).value_or(UINT32_MAX);
}

std::string Dictionary::getCountry(uint32_t country_id_in_lookup) const {
   return lookupStringValue("country", country_id_in_lookup).value();
}

uint32_t Dictionary::getRegionIdInLookup(const std::string& region) const {
   return lookupValueId("region", region).value_or(UINT32_MAX);
}

std::string Dictionary::getRegion(uint32_t region_lookup_id) const {
   return lookupStringValue("region", region_lookup_id).value();
}

uint32_t Dictionary::getPangoLineageCount() const {
   return pango_lineage_lookup.at("pango_lineage").size();
}
uint32_t Dictionary::getCountryCount() const {
   return string_lookup.at("country").size();
}
uint32_t Dictionary::getRegionCount() const {
   return string_lookup.at("region").size();
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
uint64_t Dictionary::getIdInGeneralLookup(const std::string& /*region_id_in_lookup*/) const {
   return UINT32_MAX;
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Dictionary::getGeneralLookup(uint64_t /*general_id_in_lookup*/) const {
   return "";
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
uint32_t Dictionary::getColumnIdInLookup(const std::string& /*column_name*/) const {
   return UINT32_MAX;
}
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Dictionary::getColumn(uint32_t /*column_id_in_lookup*/) const {
   return "";
}

std::optional<ValueId> Dictionary::lookupValueId(
   const std::string& column_name,
   const std::string& value
) const {
   if (string_lookup.at(column_name).contains(value)) {
      return string_lookup.at(column_name).at(value);
   }
   return std::nullopt;
};

std::optional<std::string> Dictionary::lookupStringValue(
   const std::string& column_name,
   ValueId value_id
) const {
   if (reverse_string_lookup.at(column_name).size() > value_id) {
      return reverse_string_lookup.at(column_name).at(value_id);
   }
   return std::nullopt;
}

std::optional<std::string> Dictionary::lookupPangoLineageValue(
   const std::string& column_name,
   uint32_t value_id
) const {
   if (reverse_pango_lineage_lookup.at(column_name).size() > value_id) {
      return reverse_pango_lineage_lookup.at(column_name).at(value_id);
   }
   return std::nullopt;
}

}  // namespace silo