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
      if(!getline(meta_in, header, '\n')) {
         std::cerr << "Failed to read header line. Abort." << std::endl;
         return;
      }
      std::stringstream header_in(header);
      std::string col_name;
      if(!getline(header_in, col_name, '\t') || col_name != "gisaid_epi_isl") {
         std::cerr << "Expected 'gisaid_epi_isl' as first column in metadata." << std::endl;
         return;
      }
      if(!getline(header_in, col_name, '\t') || col_name != "pango_lineage") {
         std::cerr << "Expected 'pango_lineage' as first column in metadata." << std::endl;
         return;
      }
      if(!getline(header_in, col_name, '\t') || col_name != "date") {
         std::cerr << "Expected 'date' as first column in metadata." << std::endl;
         return;
      }
      if(!getline(header_in, col_name, '\t') || col_name != "region") {
         std::cerr << "Expected 'region' as first column in metadata." << std::endl;
         return;
      }
      if(!getline(header_in, col_name, '\t') || col_name != "country") {
         std::cerr << "Expected 'country' as first column in metadata." << std::endl;
         return;
      }
      while(getline(header_in, col_name, '\t')){
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
   for (uint64_t i = 0; i < col_count; ++i) {
      dict_file << col_lookup[i] << '\t' << i << '\n';
   }
   for (uint64_t i = 0; i < general_count; ++i) {
      dict_file << general_lookup[i] << '\t' << i << '\n';
   }
}

Dictionary Dictionary::load_dict(std::istream& dict_file) {
   std::string str;
   uint32_t pango_count, region_count, country_count, col_count, dict_count;
   dict_file >> str >> pango_count >> str >> region_count >> str >> country_count >> str >> col_count >> str >> dict_count;
   Dictionary ret;
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
   uint32_t id;
   for (uint32_t i = 0; i < pango_count; ++i) {
      dict_file >> str >> id;
      ret.pango_lookup[id] = str;
      ret.pango_dict[str] = id;
   }
   for (uint32_t i = 0; i < region_count; ++i) {
      dict_file >> str >> id;
      ret.region_lookup[id] = str;
      ret.region_dict[str] = id;
   }
   for (uint32_t i = 0; i < col_count; ++i) {
      dict_file >> str >> id;
      ret.col_lookup[id] = str;
      ret.col_dict[str] = id;
   }
   for (uint32_t i = 0; i < country_count; ++i) {
      dict_file >> str >> id;
      ret.country_lookup[id] = str;
      ret.country_dict[str] = id;
   }
   for (uint64_t i = 0; i < dict_count; ++i) {
      uint64_t id64;
      dict_file >> str >> id64;
      ret.general_lookup[id64] = str;
      ret.general_dict[str] = id64;
   }
   return ret;
}

uint32_t Dictionary::get_pangoid(const std::string& str) const {
   return pango_dict.at(str);
}

const std::string& Dictionary::get_pango(uint32_t id) const {
   assert(id < pango_count);
   return pango_lookup[id];
}

uint32_t Dictionary::get_countryid(const std::string& str) const {
   return country_dict.at(str);
}

const std::string& Dictionary::get_country(uint32_t id) const {
   assert(id < country_count);
   return country_lookup[id];
}

uint32_t Dictionary::get_regionid(const std::string& str) const {
   return region_dict.at(str);
}

const std::string& Dictionary::get_region(uint32_t id) const {
   assert(id < region_count);
   return region_lookup[id];
}

uint64_t Dictionary::get_id(const std::string& str) const {
   return general_dict.at(str);
}

const std::string& Dictionary::get_str(uint64_t id) const {
   assert(id < general_count);
   return general_lookup[id];
}

uint64_t Dictionary::get_colid(const std::string& str) const {
   return col_dict.at(str);
}

const std::string& Dictionary::get_col(uint64_t id) const {
   assert(id < col_count);
   return col_lookup[id];
}