//
// Created by Alexander Taepper on 25.11.22.
//

#include <iostream>
#include <silo/common/SizeSketch.h>
#include <silo/db_components/Dictionary.h>
#include <silo/silo.h>

using namespace silo;

void Dictionary::update_dict(std::istream& meta_in, const std::unordered_map<std::string, std::string>& alias_key) {
   // Parse header. Assert order EPI, PANGO, DATE, REGION, COUNTRY, then fill additional columns
   {
      std::string header;
      if (!getline(meta_in, header, '\n')) {
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
         col_lookup.push_back(col_name);
         col_dict[col_name] = col_count++;
      }
   }

   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(meta_in, epi_isl, '\t')) break;
      if (!getline(meta_in, pango_lineage_raw, '\t')) break;
      meta_in.ignore(LONG_MAX, '\t');
      if (!getline(meta_in, region, '\t')) break;
      if (!getline(meta_in, country, '\t')) break;
      if (!getline(meta_in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(alias_key, pango_lineage_raw);

      if (!pango_dict.contains(pango_lineage)) {
         pango_lookup.push_back(pango_lineage);
         pango_dict[pango_lineage] = pango_count++;
      }
      if (!region_dict.contains(region)) {
         region_lookup.push_back(region);
         region_dict[region] = region_count++;
      }
      if (!country_dict.contains(country)) {
         country_lookup.push_back(country);
         country_dict[country] = country_count++;
      }
      if (!general_dict.contains(division)) {
         general_lookup.push_back(division);
         general_dict[division] = general_count++;
      }
   }
}

void Dictionary::save_dict(std::ostream& dict_file) const {
   dict_file << "pango_count\t" << pango_count << '\n';
   dict_file << "region_count\t" << region_count << '\n';
   dict_file << "country_count\t" << country_count << '\n';
   dict_file << "col_count\t" << col_count << '\n';
   dict_file << "dict_count\t" << general_count << '\n';
   for (uint32_t i = 0; i < pango_count; ++i) {
      dict_file << pango_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < region_count; ++i) {
      dict_file << region_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < country_count; ++i) {
      dict_file << country_lookup[i] << '\t' << i << '\n';
   }
   for (uint32_t i = 0; i < col_count; ++i) {
      dict_file << col_lookup[i] << '\t' << i << '\n';
   }
   for (uint64_t i = 0; i < general_count; ++i) {
      dict_file << general_lookup[i] << '\t' << i << '\n';
   }
}

Dictionary Dictionary::load_dict(std::istream& dict_file) {
   Dictionary ret;

   std::string str;
   uint32_t pango_count, region_count, country_count, col_count, dict_count;

   if (!getline(dict_file, str, '\t')) {
      std::cerr << "Invalid dict-header1a." << std::endl;
      return ret;
   }
   if (!getline(dict_file, str, '\n')) {
      std::cerr << "Invalid dict-header1b." << std::endl;
      return ret;
   }
   pango_count = atoi(str.c_str());

   if (!getline(dict_file, str, '\t')) {
      std::cerr << "Invalid dict-header2a." << std::endl;
      return ret;
   }
   if (!getline(dict_file, str, '\n')) {
      std::cerr << "Invalid dict-header2b." << std::endl;
      return ret;
   }
   region_count = atoi(str.c_str());

   if (!getline(dict_file, str, '\t')) {
      std::cerr << "Invalid dict-header3a." << std::endl;
      return ret;
   }
   if (!getline(dict_file, str, '\n')) {
      std::cerr << "Invalid dict-header3b." << std::endl;
      return ret;
   }
   country_count = atoi(str.c_str());

   if (!getline(dict_file, str, '\t')) {
      std::cerr << "Invalid dict-header4a." << std::endl;
      return ret;
   }
   if (!getline(dict_file, str, '\n')) {
      std::cerr << "Invalid dict-header4b." << std::endl;
      return ret;
   }
   col_count = atoi(str.c_str());

   if (!getline(dict_file, str, '\t')) {
      std::cerr << "Invalid dict-header5a." << std::endl;
      return ret;
   }
   if (!getline(dict_file, str, '\n')) {
      std::cerr << "Invalid dict-header5b." << std::endl;
      return ret;
   }
   dict_count = atoi(str.c_str());

   ret.pango_count = pango_count;
   ret.pango_lookup.resize(pango_count);
   ret.region_count = region_count;
   ret.region_lookup.resize(region_count);
   ret.country_count = country_count;
   ret.country_lookup.resize(country_count);
   ret.col_count = col_count;
   ret.col_lookup.resize(col_count);
   ret.general_count = dict_count;
   ret.general_lookup.resize(dict_count);
   std::string id_str;
   for (uint32_t i = 0; i < pango_count; ++i) {
      if (!getline(dict_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected pango_count:" << pango_count << " many lineages in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dict_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected pango_count:" << pango_count << " many lineages in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.pango_lookup[id] = str;
      ret.pango_dict[str] = id;
   }
   for (uint32_t i = 0; i < region_count; ++i) {
      if (!getline(dict_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected region_count:" << region_count << " many regions in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dict_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected region_count:" << region_count << " many regions in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.region_lookup[id] = str;
      ret.region_dict[str] = id;
   }
   for (uint32_t i = 0; i < country_count; ++i) {
      if (!getline(dict_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected country_count:" << country_count << " many countries in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dict_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected country_count:" << country_count << " many countries in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.country_lookup[id] = str;
      ret.country_dict[str] = id;
   }
   for (uint32_t i = 0; i < col_count; ++i) {
      if (!getline(dict_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected col_count:" << col_count << " many columns in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dict_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected col_count:" << col_count << " many columns in the dict file. No id" << std::endl;
         return ret;
      }
      uint32_t id = atoi(id_str.c_str());
      ret.col_lookup[id] = str;
      ret.col_dict[str] = id;
   }
   for (uint64_t i = 0; i < dict_count; ++i) {
      if (!getline(dict_file, str, '\t')) {
         std::cerr << "Unexpected end of file. Expected dict_count:" << dict_count << " many lookups in the dict file. No str" << std::endl;
         return ret;
      }
      if (!getline(dict_file, id_str, '\n')) {
         std::cerr << "Unexpected end of file. Expected dict_count:" << dict_count << " many lookups in the dict file. No id" << std::endl;
         return ret;
      }
      uint64_t id64 = atoi(id_str.c_str());
      ret.general_lookup[id64] = str;
      ret.general_dict[str] = id64;
   }
   return ret;
}

uint32_t Dictionary::get_pangoid(const std::string& str) const {
   if (pango_dict.contains(str)) {
      return pango_dict.at(str);
   } else {
      return UINT32_MAX;
   }
}

std::string error_string = "NOID";

const std::string& Dictionary::get_pango(uint32_t id) const {
   if (id == UINT32_MAX) {
      return error_string;
   }
   assert(id < pango_count);
   return pango_lookup[id];
}

uint32_t Dictionary::get_countryid(const std::string& str) const {
   if (country_dict.contains(str)) {
      return country_dict.at(str);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::get_country(uint32_t id) const {
   if (id == UINT32_MAX) {
      return error_string;
   }
   assert(id < country_count);
   return country_lookup[id];
}

uint32_t Dictionary::get_regionid(const std::string& str) const {
   if (region_dict.contains(str)) {
      return region_dict.at(str);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::get_region(uint32_t id) const {
   if (id == UINT32_MAX) {
      return error_string;
   }
   assert(id < region_count);
   return region_lookup[id];
}

uint64_t Dictionary::get_id(const std::string& str) const {
   if (general_dict.contains(str)) {
      return general_dict.at(str);
   } else {
      return UINT64_MAX;
   }
}

const std::string& Dictionary::get_str(uint64_t id) const {
   if (id == UINT64_MAX) {
      return error_string;
   }
   assert(id < general_count);
   return general_lookup[id];
}

uint32_t Dictionary::get_colid(const std::string& str) const {
   if (col_dict.contains(str)) {
      return col_dict.at(str);
   } else {
      return UINT32_MAX;
   }
}

const std::string& Dictionary::get_col(uint32_t id) const {
   if (id == UINT32_MAX) {
      return error_string;
   }
   assert(id < col_count);
   return col_lookup[id];
}
