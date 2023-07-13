#include "silo/storage/pango_lineage_alias.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <system_error>
#include <utility>

#include <spdlog/spdlog.h>
#include <nlohmann/detail/iterators/iter_impl.hpp>
#include <nlohmann/detail/iterators/iteration_proxy.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "silo/common/pango_lineage.h"

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

/*std::vector<common::UnaliasedPangoLineage> PangoLineageAliasLookup::allUnaliasedPangoLineages(
   const common::UnaliasedPangoLineage& pango_lineage
) const {
   std::vector<common::UnaliasedPangoLineage> result;
   std::vector<common::UnaliasedPangoLineage> queue;
   queue.emplace_back(pango_lineage.value);
   while (!queue.empty()) {
      common::UnaliasedPangoLineage current_lineage = std::move(queue.back());
      queue.pop_back();

      std::string pango_lineage_prefix;
      std::stringstream pango_lineage_stream(current_lineage.value);
      getline(pango_lineage_stream, pango_lineage_prefix, '.');
      if (alias_key.contains(pango_lineage_prefix)) {
         if (alias_key.at(pango_lineage_prefix).empty()) {
            result.emplace_back(std::move(current_lineage));
         }
         std::string suffix;
         if (!pango_lineage_stream.eof()) {
            suffix = std::string(
               (std::istream_iterator<char>(pango_lineage_stream)), std::istream_iterator<char>()
            );
         }
         for (const std::string& prefix : alias_key.at(pango_lineage_prefix)) {
            queue.push_back({prefix + "." + suffix});
         }
      } else {
         result.emplace_back(current_lineage);
      }
   }
   return result;
}*/

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
         "Alias key file " + pango_lineage_alias_file.relative_path().string() + " does not exist",
         std::error_code()
      );
   }

   if (pango_lineage_alias_file.extension() != ".json") {
      throw std::filesystem::filesystem_error(
         "Alias key file " + pango_lineage_alias_file.relative_path().string() +
            " is not a json file",
         std::error_code()
      );
   }

   SPDLOG_INFO(
      "Read pango lineage alias from file: {}", pango_lineage_alias_file.relative_path().string()
   );

   return PangoLineageAliasLookup(readFromJson(pango_lineage_alias_file));
}

}  // namespace silo
