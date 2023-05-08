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

void Dictionary::updateStringLookup(const std::string& column_name, const std::string& value) {
   if (!stringLookup[column_name].contains(value)) {
      reverseStringLookup[column_name].push_back(value);
      stringLookup[column_name][value] = stringLookup[column_name].size();
   }
}

Dictionary::Dictionary(const std::vector<std::string>& column_names) {
   for (const auto& column_name : column_names) {
      stringLookup[column_name] = std::unordered_map<std::string, ValueId>();
      reverseStringLookup[column_name] = std::vector<std::string>();
   }
}

Dictionary::Dictionary()
    : Dictionary(std::vector<std::string>({"country", "region", "division"})) {}

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

      if (!pango_lineage_dictionary.contains(pango_lineage)) {
         pango_lineage_lookup.push_back(pango_lineage);
         pango_lineage_dictionary[pango_lineage] = pango_lineage_count++;
      }
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
   if (pango_lineage_dictionary.contains(pango_lineage)) {
      return pango_lineage_dictionary.at(pango_lineage);
   }
   return UINT32_MAX;
}

std::string error_string = "NOID";

const std::string& Dictionary::getPangoLineage(uint32_t pango_lineage_id_in_lookup) const {
   if (pango_lineage_id_in_lookup == UINT32_MAX) {
      return error_string;
   }
   return pango_lineage_lookup.at(pango_lineage_id_in_lookup);
}

uint32_t Dictionary::getCountryIdInLookup(const std::string& country) const {
   return lookupValueId("country", country).value();
}

const std::string& Dictionary::getCountry(uint32_t country_id_in_lookup) const {
   return lookupStringValue("country", country_id_in_lookup).value_or("NOID");
}

uint32_t Dictionary::getRegionIdInLookup(const std::string& region) const {
   return lookupValueId("region", region).value();
}

const std::string& Dictionary::getRegion(uint32_t region_lookup_id) const {
   return lookupStringValue("region", region_lookup_id).value_or("NOID");
}

uint32_t Dictionary::getPangoLineageCount() const {
   return pango_lineage_count;
}
uint32_t Dictionary::getCountryCount() const {
   return stringLookup.at("country").size();
}
uint32_t Dictionary::getRegionCount() const {
   return stringLookup.at("region").size();
}

uint64_t Dictionary::getIdInGeneralLookup(const std::string& region_id_in_lookup) const {
   return 0;
}

const std::string& Dictionary::getGeneralLookup(uint64_t general_id_in_lookup) const {
   return "";
}

uint32_t Dictionary::getColumnIdInLookup(const std::string& column_name) const {
   return 0;
}

const std::string& Dictionary::getColumn(uint32_t column_id_in_lookup) const {
   return "";
}

std::optional<ValueId> Dictionary::lookupValueId(
   const std::string& column_name,
   const std::string& value
) const {
   if (stringLookup.at(column_name).contains(value)) {
      return stringLookup.at(column_name).at(value);
   }
   return std::nullopt;
};

std::optional<std::string> Dictionary::lookupStringValue(
   const std::string& column_name,
   ValueId value_id
) const {
   if (reverseStringLookup.at(column_name).size() > value_id) {
      return reverseStringLookup.at(column_name).at(value_id);
   }
   return std::nullopt;
}

}  // namespace silo