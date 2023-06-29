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
   storage::column::StringColumnPartition& column
) {
   columns.string_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IndexedStringColumnPartition& column
) {
   columns.indexed_string_columns.insert({std::string(name), column});
}
void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IntColumnPartition& column
) {
   columns.int_columns.insert({std::string(name), column});
}
void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::DateColumnPartition& column
) {
   columns.date_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::PangoLineageColumnPartition& column
) {
   columns.pango_lineage_columns.insert({std::string(name), column});
}

}  // namespace silo
