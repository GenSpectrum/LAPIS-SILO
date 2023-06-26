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
#include "silo/config/database_config.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/int_column.h"
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
      // clang-format off
      archive& raw_string_columns;
      archive& indexed_string_columns;
      archive& int_columns;
      archive& float_columns;
      archive& date_columns;
      archive& pango_lineage_columns;
      // clang-format on
   }

   std::unordered_map<std::string, storage::column::StringColumnPartition> string_columns;
   std::unordered_map<std::string, storage::column::IndexedStringColumnPartition>
      indexed_string_columns;
   std::unordered_map<std::string, storage::column::IntColumnPartition> int_columns;
   std::unordered_map<std::string, storage::column::DateColumnPartition> date_columns;
   std::unordered_map<std::string, storage::column::PangoLineageColumnPartition>
      pango_lineage_columns;
   std::unordered_map<std::string, storage::column::FloatColumnPartition> float_columns;

   unsigned fill(
      const std::filesystem::path& input_file,
      const PangoLineageAliasLookup& alias_key,
      const silo::config::DatabaseConfig& database_config
   );
};

}  // namespace silo

#endif  // SILO_META_STORE_H
