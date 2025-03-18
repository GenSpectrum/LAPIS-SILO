#pragma once

#include "silo/storage/column_group.h"
#include "silo/storage/table_partition.h"

namespace silo::storage {

class Table {
   std::vector<std::shared_ptr<TablePartition>> partitions;

  public:
   schema::TableSchema schema;

   Table(schema::TableSchema schema)
       : schema(std::move(schema)) {}

   size_t getNumberOfPartitions() const;

   const TablePartition& getPartition(size_t partition_idx) const;

   std::shared_ptr<TablePartition> addPartition();

   void validate() const;

   void loadData(const std::filesystem::path& save_directory);
   void saveData(const std::filesystem::path& save_directory);
   void validatePrimaryKeyUnique() const;
};

}  // namespace silo::storage
