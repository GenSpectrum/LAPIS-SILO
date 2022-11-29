//
// Created by Alexander Taepper on 25.11.22.
//

#ifndef SILO_DICTIONARY_H
#define SILO_DICTIONARY_H

#include <silo/common/fix_rh_map.hpp>
#include <unordered_map>

class Dictionary {
   private:
   std::unordered_map<std::string, uint32_t> pango_dict;
   std::unordered_map<std::string, uint32_t> country_dict;
   std::unordered_map<std::string, uint32_t> region_dict;
   std::unordered_map<std::string, uint64_t> general_dict;

   std::vector<std::string> pango_lookup;
   std::vector<std::string> country_lookup;
   std::vector<std::string> region_lookup;
   std::vector<std::string> general_lookup;

   uint32_t pango_count;
   uint32_t country_count;
   uint32_t region_count;
   uint64_t general_count;

   public:
   void update_dict(std::istream& dict_file, const std::unordered_map<std::string, std::string>& alias_key);

   void save_dict(std::ostream& dict_file) const;

   static Dictionary load_dict(std::istream& dict_file);

   uint32_t get_pangoid(const std::string& str) const;

   const std::string& get_pango(uint32_t id) const;

   uint32_t get_countryid(const std::string& str) const;

   const std::string& get_country(uint32_t id) const;

   uint32_t get_regionid(const std::string& str) const;

   const std::string& get_region(uint32_t id) const;

   uint64_t get_id(const std::string& str) const;

   const std::string& get_str(uint64_t id) const;
};

#endif //SILO_DICTIONARY_H
