#include "silo/storage/pango_lineage_alias.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <system_error>
#include <utility>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/pango_lineage.h"
#include "silo/common/string_utils.h"

namespace silo {

PangoLineageAliasLookup::PangoLineageAliasLookup(
   std::unordered_map<std::string, std::vector<std::string>> alias_key
)
    : alias_key(std::move(alias_key)) {}

common::UnaliasedPangoLineage PangoLineageAliasLookup::unaliasPangoLineage(
   const common::RawPangoLineage& pango_lineage
) const {
   std::string pango_lineage_prefix;
   std::stringstream pango_lineage_stream(pango_lineage.value);
   getline(pango_lineage_stream, pango_lineage_prefix, '.');
   if (alias_key.contains(pango_lineage_prefix)) {
      if (alias_key.at(pango_lineage_prefix).empty()) {
         return {pango_lineage.value};
      }
      if (alias_key.at(pango_lineage_prefix).size() > 1) {
         return {pango_lineage.value};
      }
      if (pango_lineage_stream.eof()) {
         return {alias_key.at(pango_lineage_prefix).at(0)};
      }
      const std::string suffix(
         (std::istream_iterator<char>(pango_lineage_stream)), std::istream_iterator<char>()
      );
      return {alias_key.at(pango_lineage_prefix).at(0) + '.' + suffix};
   }
   return {pango_lineage.value};
}

common::AliasedPangoLineage PangoLineageAliasLookup::aliasPangoLineage(
   const common::UnaliasedPangoLineage& pango_lineage
) const {
   const auto elements = splitBy(pango_lineage.value, ".");
   const size_t num_elements = elements.size();

   for (auto i = num_elements; i > 3; i--) {
      const auto search_value = boost::join(slice(elements, 0, i - 1), ".");

      for (const auto& alias_entry : alias_key) {
         const auto alias = alias_entry.first;
         const auto alias_values = alias_entry.second;

         if (alias_values.size() > 1 || alias_values.empty()) {
            continue;
         }

         if (alias_values.at(0) == search_value) {
            const auto leftover_value = boost::join(slice(elements, i - 1, num_elements), ".");

            std::string value = alias;

            if (!leftover_value.empty()) {
               value += "." + leftover_value;
            }
            return {value};
         }
      }
   }

   return {pango_lineage.value};
}

namespace {

std::unordered_map<std::string, std::vector<std::string>> readFromJson(
   const std::filesystem::path& pango_lineage_alias_file
) {
   std::unordered_map<std::string, std::vector<std::string>> alias_keys;
   nlohmann::json alias_key_json;
   std::ifstream(pango_lineage_alias_file) >> alias_key_json;

   for (const auto& [key, value] : alias_key_json.items()) {
      if (value.is_array()) {
         alias_keys[key] = value.get<std::vector<std::string>>();
      } else if (value.is_string() && !value.get<std::string>().empty()) {
         alias_keys[key] = {value.get<std::string>()};
      }
   }
   return alias_keys;
}
}  // namespace

silo::PangoLineageAliasLookup silo::PangoLineageAliasLookup::readFromFile(
   const std::filesystem::path& pango_lineage_alias_file
) {
   if (!std::filesystem::exists(pango_lineage_alias_file)) {
      throw std::filesystem::filesystem_error(
         "Alias key file " + pango_lineage_alias_file.string() + " does not exist",
         std::error_code()
      );
   }

   if (pango_lineage_alias_file.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Alias key file " + pango_lineage_alias_file.string() + " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO("Read pango lineage alias from file: {}", pango_lineage_alias_file.string());

   return PangoLineageAliasLookup(readFromJson(pango_lineage_alias_file));
}

}  // namespace silo
