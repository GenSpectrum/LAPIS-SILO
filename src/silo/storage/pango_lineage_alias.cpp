#include "silo/storage/pango_lineage_alias.h"

#include <csv.hpp>
#include <filesystem>
#include <iterator>
#include <sstream>

namespace silo {

const std::string PANGO_ALIAS_FILENAME = "pango_alias.txt";

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

silo::PangoLineageAliasLookup silo::PangoLineageAliasLookup::readFromFile(const std::string& working_directory) {
   std::filesystem::path const alias_key_path(working_directory + PANGO_ALIAS_FILENAME);
   if (!std::filesystem::exists(alias_key_path)) {
      throw std::filesystem::filesystem_error(
         "Alias key file " + alias_key_path.relative_path().string() + " does not exist",
         std::error_code()
      );
   }

   std::unordered_map<std::string, std::string> alias_keys;

   csv::CSVFormat format;
   format.no_header().delimiter('\t');

   csv::CSVReader csv_reader{alias_key_path.string(), format};
   for (const auto& row : csv_reader) {
      const std::string alias = row[0].get();
      const std::string val = row[1].get();
      alias_keys[alias] = val;
   }

   return alias_keys;
}

}  // namespace silo
