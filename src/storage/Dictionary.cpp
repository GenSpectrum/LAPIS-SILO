#include <silo/common/silo_symbols.h>
#include <silo/database.h>
#include <silo/storage/Dictionary.h>
#include <cassert>
#include <iostream>

using namespace silo;

void Dictionary::updateDictionary(
   std::istream& dictionary_file,
   const std::unordered_map<std::string, std::string>& alias_key
) {
   // Parse header. Assert order EPI, PANGO, DATE, REGION, COUNTRY, then fill additional columns
   {
      std::string header;
      if (!getline(dictionary_file, header, '\n')) {
         std::cerr << "Failed to read header line. Abort." << std::endl;
         return;
      }
      std::stringstream header_in(header);
      std::string col_name;
      if (!getline(header_in, col_name, '\t') || col_name != "gisaid_epi_isl") {
         std::cerr << "Expected 'gisaid_epi_isl' as first column in metadata." << std::endl;
         return;
      }
      if (!getline(header_in, col_name, '\t') || col_name != "pango_lineage") {
         std::cerr << "Expected 'pango_lineage' as first column in metadata." << std::endl;
         return;
      }
      if (!getline(header_in, col_name, '\t') || col_name != "date") {
         std::cerr << "Expected 'date' as first column in metadata." << std::endl;
         return;
      }
      if (!getline(header_in, col_name, '\t') || col_name != "region") {
         std::cerr << "Expected 'region' as first column in metadata." << std::endl;
         return;
      }
      if (!getline(header_in, col_name, '\t') || col_name != "country") {
         std::cerr << "Expected 'country' as first column in metadata." << std::endl;
         return;
      }
      while (!header_in.eof()) {
         getline(header_in, col_name, '\t');
         additional_columns_lookup.push_back(col_name);
         additional_columns_dictionary[col_name] = additional_columns_count++;
      }
   }

   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(dictionary_file, epi_isl, '\t'))
         break;
      if (!getline(dictionary_file, pango_lineage_raw, '\t'))
         break;
      dictionary_file.ignore(LONG_MAX, '\t');
      if (!getline(dictionary_file, region, '\t'))
         break;
      if (!getline(dictionary_file, country, '\t'))
         break;
      if (!getline(dictionary_file, division, '\n'))
         break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolvePangoLineageAlias(alias_key, pango_lineage_raw);

      if (!pango_lineage_dictionary.contains(pango_lineage)) {
         pango_lineage_lookup.push_back(pango_lineage);
         pango_lineage_dictionary[pango_lineage] = pango_lineage_count++;
      }
      if (!region_dictionary.contains(region)) {
         region_lookup.push_back(region);
         region_dictionary[region] = region_count++;
      }
      if (!country_dictionary.contains(country)) {
         country_lookup.push_back(country);
         country_dictionary[country] = country_count++;
      }
      if (!general_dictionary.contains(division)) {
         general_lookup.push_back(division);
         general_dictionary[division] = general_count++;
      }
   }
}

void Dictionary::saveDictionary(std::ostream& dictionary_file) const {
   dictionary_file << "pango_lineage_count\t" << pango_lineage_count << '\n';
   dictionary_file << "region_count\t" << region_count << '\n';
   dictionary_file << "country_count\t" << country_count << '\n';
   dictionary_file << "additional_columns_count\t" << additional_columns_count << '\n';
   dictionary_file << "dict_count\t" << general_count << '\n';
   for (uint32_t i = 0; i < pango_lineage_count; ++i) {
      dictionary_file << pango_lineage_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < region_count; ++i) {
      dictionary_file << region_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < country_count; ++i) {
      dictionary_file << country_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < additional_columns_count; ++i) {
      dictionary_file << additional_columns_lookup[i] << '\t' << i << '\n';
   }
   for (uint64_t i = 0; i < general_count; ++i) {
      dictionary_file << general_lookup[i] << '\t' << i << '\n';
   }
}

Dictionary Dictionary::loadDictionary(std::istream& dictionary_file) {
   Dictionary ret;

   std::string str;
   uint32_t pango_count, region_count, country_count, col_count, dict_count;

   if (!getline(dictionary_file, str, '\t')) {
      std::cerr << "Invalid dict-header1a." << std::endl;
      return ret;
   }
   if (!getline(dictionary_file, str, '\n')) {
      std::cerr << "Invalid dict-header1b." << std::endl;
      return ret;
   }
   pango_count = atoi(str.c_str());

   if (!getline(dictionary_file, str, '\t')) {
      std::cerr << "Invalid dict-header2a." << std::endl;
      return ret;
   }
   if (!getline(dictionary_file, str, '\n')) {
      std::cerr << "Invalid dict-header2b." << std::endl;
      return ret;
   }
   region_count = atoi(str.c_str());

   if (!getline(dictionary_file, str, '\t')) {
      std::cerr << "Invalid dict-header3a." << std::endl;
      return ret;
   }
   if (!getline(dictionary_file, str, '\n')) {
      std::cerr << "Invalid dict-header3b." << std::endl;
      return ret;
   }
   country_count = atoi(str.c_str());

   if (!getline(dictionary_file, str, '\t')) {
      std::cerr << "Invalid dict-header4a." << std::endl;
      return ret;
   }
   if (!getline(dictionary_file, str, '\n')) {
      std::cerr << "Invalid dict-header4b." << std::endl;
      return ret;
   }
   col_count = atoi(str.c_str());

   if (!getline(dictionary_file, str, '\t')) {
      std::cerr << "Invalid dict-header5a." << std::endl;
      return ret;
   }
   if (!getline(dictionary_file, str, '\n')) {
      std::cerr << "Invalid dict-header5b." << std::endl;
      return ret;
   }
   dict_count = atoi(str.c_str());

   ret.pango_lineage_count = pango_count;
   ret.pango_lineage_lookup.resize(pango_count);
   ret.region_count = region_count;
   ret.region_lookup.resize(region_count);
   ret.country_count = country_count;
   ret.country_lookup.resize(country_count);
   ret.additional_columns_count = col_count;
   ret.additional_columns_lookup.resize(col_count);
   ret.general_count = dict_count;
   ret.general_lookup.resize(dict_count);
   std::string id_str;
   for (uint32_t i = 0; i < pango_count; ++i) {
      if (!getline(dictionary_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected pango_lineage_count:" << pango_count
                   << " many lineages in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dictionary_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected pango_lineage_count:" << pango_count
                   << " many lineages in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.pango_lineage_lookup[id] = str;
      ret.pango_lineage_dictionary[str] = id;
   }
   for (uint32_t i = 0; i < region_count; ++i) {
      if (!getline(dictionary_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected region_count:" << region_count
                   << " many regions in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dictionary_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected region_count:" << region_count
                   << " many regions in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.region_lookup[id] = str;
      ret.region_dictionary[str] = id;
   }
   for (uint32_t i = 0; i < country_count; ++i) {
      if (!getline(dictionary_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected country_count:" << country_count
                   << " many countries in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dictionary_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected country_count:" << country_count
                   << " many countries in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.country_lookup[id] = str;
      ret.country_dictionary[str] = id;
   }
   for (uint32_t i = 0; i < col_count; ++i) {
      if (!getline(dictionary_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected additional_columns_count:" << col_count
                   << " many columns in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dictionary_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected additional_columns_count:" << col_count
                   << " many columns in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.additional_columns_lookup[id] = str;
      ret.additional_columns_dictionary[str] = id;
   }
   for (uint64_t i = 0; i < dict_count; ++i) {
      if (!getline(dictionary_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected dict_count:" << dict_count
                   << " many lookups in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dictionary_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected dict_count:" << dict_count
                   << " many lookups in the dict file. No id" << std::endl;
         return ret;
      }
      uint64_t id64 = atoi(id_str.c_str());
      ret.general_lookup[id64] = str;
      ret.general_dictionary[str] = id64;
   }
   return ret;
}

uint32_t Dictionary::getPangoLineageIdInLookup(const std::string& pango_lineage) const {
   if (pango_lineage_dictionary.contains(pango_lineage)) {
      return pango_lineage_dictionary.at(pango_lineage);
   } else {
      return UINT32_MAX;
   }
}

std::string error_string = "NOID";

const std::string& Dictionary::getPangoLineage(uint32_t pango_lineage_id_in_lookup) const {
   if (pango_lineage_id_in_lookup == UINT32_MAX) {
      return error_string;
   }
   assert(pango_lineage_id_in_lookup < pango_lineage_count);
   return pango_lineage_lookup[pango_lineage_id_in_lookup];
}

uint32_t Dictionary::getCountryIdInLookup(const std::string& country) const {
   if (country_dictionary.contains(country)) {
      return country_dictionary.at(country);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::getCountry(uint32_t country_id_in_lookup) const {
   if (country_id_in_lookup == UINT32_MAX) {
      return error_string;
   }
   assert(country_id_in_lookup < country_count);
   return country_lookup[country_id_in_lookup];
}

uint32_t Dictionary::getRegionIdInLookup(const std::string& region) const {
   if (region_dictionary.contains(region)) {
      return region_dictionary.at(region);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::getRegion(uint32_t id) const {
   if (id == UINT32_MAX) {
      return error_string;
   }
   assert(id < region_count);
   return region_lookup[id];
}

uint64_t Dictionary::getIdInGeneralLookup(const std::string& region_id_in_lookup) const {
   if (general_dictionary.contains(region_id_in_lookup)) {
      return general_dictionary.at(region_id_in_lookup);
   } else {
      return UINT64_MAX;
   }
}

const std::string& Dictionary::getGeneralLookup(uint64_t general_id_in_lookup) const {
   if (general_id_in_lookup == UINT64_MAX) {
      return error_string;
   }
   assert(general_id_in_lookup < general_count);
   return general_lookup[general_id_in_lookup];
}

uint32_t Dictionary::getColumnIdInLookup(const std::string& column_name) const {
   if (additional_columns_dictionary.contains(column_name)) {
      return additional_columns_dictionary.at(column_name);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::getColumn(uint32_t column_id_in_lookup) const {
   if (column_id_in_lookup == UINT32_MAX) {
      return error_string;
   }
   assert(column_id_in_lookup < additional_columns_count);
   return additional_columns_lookup[column_id_in_lookup];
}
uint32_t Dictionary::getPangoLineageCount() const {
   return pango_lineage_count;
}
uint32_t Dictionary::getCountryCount() const {
   return country_count;
}
uint32_t Dictionary::getRegionCount() const {
   return region_count;
}
