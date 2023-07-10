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

namespace silo {

PangoLineageAliasLookup::PangoLineageAliasLookup(
   std::unordered_map<std::string, std::string> alias_key
)
    : alias_key(std::move(alias_key)) {}

std::string PangoLineageAliasLookup::resolvePangoLineageAlias(const std::string& pango_lineage
) const {
   std::string pango_lineage_prefix;
   std::stringstream pango_lineage_stream(pango_lineage);
   getline(pango_lineage_stream, pango_lineage_prefix, '.');
   if (alias_key.contains(pango_lineage_prefix)) {
      if (alias_key.at(pango_lineage_prefix).empty()) {
         return pango_lineage;
      }
      if (pango_lineage_stream.eof()) {
         return alias_key.at(pango_lineage_prefix);
      }
      const std::string suffix(
         (std::istream_iterator<char>(pango_lineage_stream)), std::istream_iterator<char>()
      );
      return alias_key.at(pango_lineage_prefix) + '.' + suffix;
   }
   return pango_lineage;
}

namespace {

std::unordered_map<std::string, std::string> readFromJson(
   const std::filesystem::path& pango_lineage_alias_file
) {
   std::unordered_map<std::string, std::string> alias_keys;
   nlohmann::json alias_key_json;
   std::ifstream(pango_lineage_alias_file) >> alias_key_json;

   for (const auto& [key, value] : alias_key_json.items()) {
      if (value.is_array()) {
         SPDLOG_INFO(
            "Alias value {} is a recombinant. Recombinants are not implemented yet.", value.dump()
         );
         continue;
      }
      alias_keys[key] = value;
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
