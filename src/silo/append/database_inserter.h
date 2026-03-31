#pragma once

#include <memory>
#include <utility>

#include <simdjson.h>
#include <spdlog/spdlog.h>

#include "silo/append/ndjson_line_reader.h"
#include "silo/storage/table.h"

namespace silo::append {

class TableInserter {
   std::shared_ptr<storage::Table> table;

  public:
   class Commit {
      friend class TableInserter;

      Commit() = default;
   };

   explicit TableInserter(std::shared_ptr<storage::Table> table)
       : table(std::move(table)) {}

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

TableInserter::Commit appendDataToTable(
   std::shared_ptr<silo::storage::Table> table,
   NdjsonLineReader& input_data
);

}  // namespace silo::append
