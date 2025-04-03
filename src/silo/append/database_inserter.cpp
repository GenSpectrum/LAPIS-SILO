#include "silo/append/database_inserter.h"

namespace silo::append {

void TablePartitionInserter::insert(const nlohmann::json& ndjson_line) {
   for (auto& column_metadata : table_partition->columns.metadata) {
      try {
         table_partition->columns.addJsonValueToColumn(column_metadata, ndjson_line);
      } catch (const nlohmann::json::type_error& error) {
         throw silo::append::AppendException(
            "The following line does not conform to SILO's json specification error when adding "
            "to database column {}: '{}'\n"
            "json type_error: {}",
            column_metadata.name,
            ndjson_line.dump(),
            error.what()
         );
      }
   }
   table_partition->sequence_count++;
}

TablePartitionInserter::Commit TablePartitionInserter::commit() {
   table_partition->finalize();
   table_partition->validate();
   return Commit{};
}

TablePartitionInserter TableInserter::openNewPartition() {
   return TablePartitionInserter{table->addPartition()};
}

TableInserter::Commit TableInserter::commit() {
   table->validate();
   return Commit{};
}

}  // namespace silo::append
