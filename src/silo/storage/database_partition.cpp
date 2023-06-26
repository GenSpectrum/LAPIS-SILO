#include "silo/storage/database_partition.h"

#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"

namespace silo {

const std::vector<preprocessing::Chunk>& DatabasePartition::getChunks() const {
   return chunks;
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::StringColumnPartition column
) {
   meta_store.string_columns.insert(
      std::make_pair<std::string, storage::column::StringColumnPartition>(
         std::string(name), std::move(column)
      )
   );
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IndexedStringColumnPartition column
) {
   meta_store.indexed_string_columns.insert(
      std::make_pair<std::string, storage::column::IndexedStringColumnPartition>(
         std::string(name), std::move(column)
      )
   );
}
void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IntColumnPartition column
) {
   meta_store.int_columns.insert(std::make_pair<std::string, storage::column::IntColumnPartition>(
      std::string(name), std::move(column)
   ));
}
void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::DateColumnPartition column
) {
   meta_store.date_columns.insert(std::make_pair<std::string, storage::column::DateColumnPartition>(
      std::string(name), std::move(column)
   ));
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::PangoLineageColumnPartition column
) {
   meta_store.pango_lineage_columns.insert(
      std::make_pair<std::string, storage::column::PangoLineageColumnPartition>(
         std::string(name), std::move(column)
      )
   );
}

}  // namespace silo
