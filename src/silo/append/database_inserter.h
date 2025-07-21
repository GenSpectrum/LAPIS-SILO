#pragma once

#include <memory>

#include <spdlog/spdlog.h>

#include "evobench/evobench.hpp"
#include "silo/append/append_exception.h"
#include "silo/database.h"
#include "silo/storage/table_partition.h"

namespace silo::append {

class TablePartitionInserter {
   std::shared_ptr<storage::TablePartition> table_partition;

  public:
   class Commit {
      friend class TablePartitionInserter;

      Commit() = default;
   };

   TablePartitionInserter(std::shared_ptr<storage::TablePartition> table_partition)
       : table_partition(table_partition) {}

   void insert(const nlohmann::json& ndjson_line) const;

   Commit commit() const;
};

class TableInserter {
   std::shared_ptr<storage::Table> table;

  public:
   class Commit {
      friend class TableInserter;

      Commit() = default;
   };

   TableInserter(std::shared_ptr<storage::Table> table)
       : table(table) {}

   TablePartitionInserter openNewPartition() const;

   Commit commit() const;
};

template <std::ranges::range Data>
   requires std::same_as<std::ranges::range_value_t<Data>, nlohmann::json>
silo::append::TablePartitionInserter::Commit appendDataToTablePartition(
   silo::append::TablePartitionInserter partition_inserter,
   Data input_data
) {
   EVOBENCH_SCOPE("TablePartitionInserter", "appendDataToTablePartition");
   size_t line_count = 0;
   for (const auto& json_obj : input_data) {
      partition_inserter.insert(json_obj);

      line_count++;
      if (line_count % 10000 == 0) {
         SPDLOG_INFO("Processed {} json objects from the input file", line_count);
      }
   }

   return partition_inserter.commit();
}

template <std::ranges::range Data>
   requires std::same_as<std::ranges::range_value_t<Data>, nlohmann::json>
silo::append::TableInserter::Commit appendDataToTable(
   std::shared_ptr<silo::storage::Table> table,
   Data input_data
) {
   TableInserter table_inserter(table);

   // TODO(#738) make partition configurable
   auto table_partition = table_inserter.openNewPartition();

   appendDataToTablePartition(table_partition, input_data);

   return table_inserter.commit();
}

template <std::ranges::range Data>
   requires std::same_as<std::ranges::range_value_t<Data>, nlohmann::json>
void appendDataToDatabase(Database& database, Data input_data) {
   appendDataToTable(database.table, input_data);
   database.updateDataVersion();
}

}  // namespace silo::append