#pragma once

#include <memory>

#include <simdjson.h>
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

   struct SniffedField {
      silo::schema::ColumnIdentifier column_identifier;
      // Looking up keys in their escaped form is fastest.
      // As fallback in case we do not find the key in the escaped form
      // in subsequent json elements, will unescape the key.
      std::string escaped_key;
   };

   // Inserting is faster if we parse the fields in the correct order.
   // Sniff the order from the first json in the ndjson stream
   std::vector<SniffedField> sniffFieldOrder(simdjson::ondemand::document& ndjson_line) const;

   void insert(
      simdjson::ondemand::document& ndjson_line,
      const std::vector<SniffedField>& field_order_hint
   ) const;

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

template <typename Data>
   requires std::same_as<
      typename Data::iterator::value_type,
      simdjson::simdjson_result<simdjson::ondemand::document>>
silo::append::TablePartitionInserter::Commit appendDataToTablePartition(
   silo::append::TablePartitionInserter partition_inserter,
   Data& input_data
) {
   EVOBENCH_SCOPE("TablePartitionInserter", "appendDataToTablePartition");
   size_t line_count = 0;
   for (simdjson::simdjson_result<simdjson::ondemand::document_reference> json_obj_or_error :
        input_data) {
      simdjson::ondemand::document_reference ndjson_line;
      auto error = json_obj_or_error.get(ndjson_line);
      if (error) {
         throw silo::append::AppendException("err: {}", simdjson::error_message(error));
      }
      auto sniffed_field_order = partition_inserter.sniffFieldOrder(ndjson_line);
      partition_inserter.insert(ndjson_line, sniffed_field_order);

      line_count++;
      if (line_count % 10000 == 0) {
         SPDLOG_INFO("Processed {} json objects from the input file", line_count);
      }
   }

   return partition_inserter.commit();
}

template <typename Data>
   requires std::same_as<
      typename Data::iterator::value_type,
      simdjson::simdjson_result<simdjson::ondemand::document>>
silo::append::TableInserter::Commit appendDataToTable(
   std::shared_ptr<silo::storage::Table> table,
   Data& input_data
) {
   TableInserter table_inserter(table);

   // TODO(#738) make partition configurable
   auto table_partition = table_inserter.openNewPartition();

   appendDataToTablePartition(table_partition, input_data);

   return table_inserter.commit();
}

template <typename Data>
   requires std::same_as<
      typename Data::iterator::value_type,
      simdjson::simdjson_result<simdjson::ondemand::document>>
void appendDataToDatabase(Database& database, Data& input_data) {
   appendDataToTable(database.table, input_data);
   database.updateDataVersion();
}

}  // namespace silo::append