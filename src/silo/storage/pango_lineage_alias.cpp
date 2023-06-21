#include "silo/storage/pango_lineage_alias.h"

#include <spdlog/spdlog.h>
#include <csv.hpp>
#include <filesystem>
#include <iterator>
#include <sstream>

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

silo::PangoLineageAliasLookup silo::PangoLineageAliasLookup::readFromFile(
   const std::filesystem::path& pango_lineage_alias_file
) {
   if (!std::filesystem::exists(pango_lineage_alias_file)) {
      throw std::filesystem::filesystem_error(
         "Alias key file " + pango_lineage_alias_file.relative_path().string() + " does not exist",
         std::error_code()
      );
   }

   SPDLOG_INFO(
      "Read pango lineage alias from file: {}", pango_lineage_alias_file.relative_path().string()
   );
   std::unordered_map<std::string, std::string> alias_keys;

   csv::CSVFormat format;
   format.no_header().delimiter('\t');

   csv::CSVReader csv_reader{pango_lineage_alias_file.string(), format};
   for (const auto& row : csv_reader) {
      const std::string alias = row[0].get();
      const std::string val = row[1].get();
      alias_keys[alias] = val;
   }

   return PangoLineageAliasLookup(alias_keys);
}

}  // namespace silo
