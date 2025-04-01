#pragma once

#include <memory>

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

   Commit commit() {
      table_partition->finalize();
      table_partition->validate();
      return Commit{};
   }

   void insert(const nlohmann::json& ndjson_line) {
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
};

class TableInserter {
   storage::Table* table;

  public:
   class Commit {
      friend class TableInserter;

      Commit() = default;
   };

   TableInserter(storage::Table* table)
       : table(table) {}

   Commit commit() {
      table->validate();
      return Commit{};
   }

   TablePartitionInserter openNewPartition() {
      return TablePartitionInserter{table->addPartition()};
   }
};

template <std::ranges::range Data>
   requires std::same_as<std::ranges::range_value_t<Data>, nlohmann::json>
silo::append::TablePartitionInserter::Commit appendDataToTablePartition(
   silo::append::TablePartitionInserter partition_inserter,
   Data input_data
) {
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
   silo::storage::Table& table,
   Data input_data
) {
   TableInserter table_inserter(&table);

   // TODO(#738) make partition configurable
   auto table_partition = table_inserter.openNewPartition();

   appendDataToTablePartition(table_partition, input_data);

   return table_inserter.commit();
}

template <std::ranges::range Data>
   requires std::same_as<std::ranges::range_value_t<Data>, nlohmann::json>
void appendDataToDatabase(Database& database, Data input_data) {
   appendDataToTable(database.table, input_data);
}

}  // namespace silo::append