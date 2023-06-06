#ifndef SILO_META_STORE_H
#define SILO_META_STORE_H

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

#include <roaring/roaring.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"

namespace silo {

class PangoLineageAliasLookup;

namespace config {
class DatabaseConfig;
}  // namespace config

struct MetadataStore {
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& raw_string_columns;
      archive& indexed_string_columns;
      archive& date_columns;
   }

   std::unordered_map<std::string, storage::column::RawStringColumn> raw_string_columns;
   std::unordered_map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::unordered_map<std::string, storage::column::DateColumn> date_columns;
   std::unordered_map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;

   unsigned fill(
      const std::filesystem::path& input_file,
      const PangoLineageAliasLookup& alias_key,
      const silo::config::DatabaseConfig& database_config
   );

  private:
   void initializeColumns(
      const config::DatabaseConfig& database_config,
      const std::set<std::string>& columns_to_index
   );
};

}  // namespace silo

#endif  // SILO_META_STORE_H
