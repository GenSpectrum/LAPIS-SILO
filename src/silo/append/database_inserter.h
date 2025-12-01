#pragma once

#include <memory>
#include <utility>

#include <simdjson.h>
#include <spdlog/spdlog.h>

#include "silo/append/ndjson_line_reader.h"
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

   explicit TablePartitionInserter(std::shared_ptr<storage::TablePartition> table_partition)
       : table_partition(std::move(table_partition)) {}

   struct SniffedField {
      silo::schema::ColumnIdentifier column_identifier;
      // Looking up keys in their escaped form is fastest.
      // As fallback in case we do not find the key in the escaped form
      // in subsequent json elements, will unescape the key.
      std::string escaped_key;
   };

   // Inserting is faster if we parse the fields in the correct order.
   // Sniff the order from the first json in the ndjson stream
   [[nodiscard]] std::expected<std::vector<SniffedField>, std::string> sniffFieldOrder(
      simdjson::ondemand::document_reference ndjson_line
   ) const;

   [[nodiscard]] std::expected<void, std::string> insert(
      simdjson::ondemand::document_reference ndjson_line,
      const std::vector<SniffedField>& field_order_hint
   ) const;

   [[nodiscard]] Commit commit() const;
};

class TableInserter {
   std::shared_ptr<storage::Table> table;

  public:
   class Commit {
      friend class TableInserter;

      Commit() = default;
   };

   explicit TableInserter(std::shared_ptr<storage::Table> table)
       : table(std::move(table)) {}

   [[nodiscard]] TablePartitionInserter openNewPartition() const;

   [[nodiscard]] Commit commit() const;
};

TablePartitionInserter::Commit appendDataToTablePartition(
   const TablePartitionInserter& partition_inserter,
   NdjsonLineReader& input_data
);

TableInserter::Commit appendDataToTable(
   std::shared_ptr<silo::storage::Table> table,
   NdjsonLineReader& input_data
);

void appendDataToDatabase(Database& database, NdjsonLineReader& input_data);

}  // namespace silo::append