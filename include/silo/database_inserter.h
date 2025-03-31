#pragma once

#include <memory>

#include "silo/database.h"
#include "silo/storage/table_partition.h"

namespace silo {

class TablePartitionInserter {
   std::shared_ptr<storage::TablePartition> table_partition;

  public:
   TablePartitionInserter(std::shared_ptr<storage::TablePartition> table_partition)
       : table_partition(table_partition) {}

   ~TablePartitionInserter() {
      //      table_partition->validate();
      table_partition->finalize();
   }

   void insert(const nlohmann::json& ndjson_line) {
      for (auto& column_metadata : table_partition->columns.metadata) {
         try {
            table_partition->columns.addJsonValueToColumn(column_metadata, ndjson_line);
         } catch (const nlohmann::json::type_error& error) {
            throw std::runtime_error(fmt::format(
               "The following line does not conform to SILO's json specification error when adding "
               "to database column {}: '{}'",
               column_metadata.name,
               ndjson_line.dump()
            ));
         }
      }
      table_partition->sequence_count++;
   }
};

class TableInserter {
   storage::Table* table;

  public:
   TableInserter(storage::Table* table)
       : table(table) {}

   ~TableInserter() {
      //      table->validate();
   }

   TablePartitionInserter openNewPartition() {
      return TablePartitionInserter{table->addPartition()};
   }
};

}  // namespace silo