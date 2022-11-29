//
// Created by Alexander Taepper on 25.11.22.
//

#ifndef SILO_DICTIONARY_H
#define SILO_DICTIONARY_H

#include <silo/common/fix_rh_map.hpp>
#include <unordered_map>

class Dictionary {
   private:
   silo::fix_rh_map<std::string_view, uint32_t> pango_dict;
   silo::fix_rh_map<std::string_view, uint32_t> country_dict;
   silo::fix_rh_map<std::string_view, uint32_t> region_dict;
   silo::fix_rh_map<std::string_view, uint64_t> general_dict;

   std::vector<std::string> pango_lookup;
   std::vector<std::string> country_lookup;
   std::vector<std::string> region_lookup;
   std::vector<std::string> general_lookup;

   uint32_t pango_count;
   uint32_t country_count;
   uint32_t region_count;
   uint64_t dict_count;

   Dictionary(uint32_t pango_count, uint32_t country_count, uint32_t region_count, uint64_t dict_count) : pango_dict(silo::fix_rh_map<std::string_view, uint32_t>(pango_count)),
                                                                                                          country_dict(silo::fix_rh_map<std::string_view, uint32_t>(country_count)),
                                                                                                          region_dict(silo::fix_rh_map<std::string_view, uint32_t>(region_count)),
                                                                                                          general_dict(silo::fix_rh_map<std::string_view, uint64_t>(dict_count)),
                                                                                                          pango_lookup(std::vector<std::string>(pango_count)),
                                                                                                          country_lookup(std::vector<std::string>(country_count)),
                                                                                                          region_lookup(std::vector<std::string>(region_count)),
                                                                                                          general_lookup(std::vector<std::string>(dict_count)),
                                                                                                          pango_count(pango_count),
                                                                                                          country_count(country_count),
                                                                                                          region_count(region_count),
                                                                                                          dict_count(dict_count) {}

   public:
   static Dictionary build_dict(std::istream& dict_file, const std::unordered_map<std::string, std::string>& alias_key);

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
