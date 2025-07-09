#include "silo/append/database_inserter.h"

#include <algorithm>

#include "silo/schema/duplicate_primary_key_exception.h"

namespace silo::append {

namespace {

std::string contextForError(simdjson::ondemand::document& document) {
   document.rewind();
   std::string_view actual_line;
   auto error = document.raw_json().get(actual_line);
   if (error) {
      return fmt::format(
         "Error, could not get raw line from document: {}", simdjson::error_message(error)
      );
   }
   return std::string{actual_line};
}

simdjson::simdjson_result<simdjson::ondemand::value> findFieldManual(
   simdjson::ondemand::object& object,
   const silo::schema::ColumnIdentifier& column_identifier
) {
   for (auto field : object) {
      std::string_view unescaped_field;
      auto error = field.unescaped_key().get(unescaped_field);
      if (error)
         return error;
      if (unescaped_field == column_identifier.name) {
         return field.value();
      }
   }
   return simdjson::error_code::NO_SUCH_FIELD;
}

std::optional<simdjson::ondemand::value> findFieldWithFallbacks(
   simdjson::ondemand::object& object,
   const TablePartitionInserter::SniffedField& sniffed_field
) {
   simdjson::ondemand::value column_value;
   auto error = object.find_field(sniffed_field.escaped_key).get(column_value);
   if (!error) {
      return column_value;
   }
   SPDLOG_WARN(
      "Expected to find the key {} next based on sniffing of the first line. Falling back to "
      "looking "
      "through all keys"
   );
   error = object.find_field_unordered(sniffed_field.escaped_key).get(column_value);
   if (!error) {
      return column_value;
   }
   SPDLOG_WARN(
      "Also did not find the key when looking from the beginning. Falling back to also looking for "
      "the unescaped value"
   );
   error = findFieldManual(object, sniffed_field.column_identifier).get(column_value);
   if (!error) {
      return column_value;
   }
   return std::nullopt;
}

}  // namespace

std::vector<TablePartitionInserter::SniffedField> TablePartitionInserter::sniffFieldOrder(
   simdjson::ondemand::document& ndjson_line
) const {
   std::vector<SniffedField> order_in_json_line;
   auto columns_in_table = table_partition->columns.metadata;
   simdjson::ondemand::object object;
   auto error = ndjson_line.get_object().get(object);
   if (error) {
      throw silo::append::AppendException(
         "expect each ndjson line to be an object, got: {}", contextForError(ndjson_line)
      );
   }
   for (auto field : object) {
      std::string_view raw_key_sv;
      error = field.escaped_key().get(raw_key_sv);
      if (error) {
         throw silo::append::AppendException(
            "error '{}', when getting the key of a field in the following json line: {}",
            simdjson::error_message(error),
            contextForError(ndjson_line)
         );
      }
      std::string raw_key{raw_key_sv};

      std::string_view unescaped_key;
      error = field.unescaped_key().get(unescaped_key);
      if (error) {
         throw silo::append::AppendException(
            "error '{}', when unescaping the key of field {} in the following ndjson line {}",
            simdjson::error_message(error),
            raw_key,
            contextForError(ndjson_line)
         );
      }
      auto maybe_column_metadata =
         std::find_if(columns_in_table.begin(), columns_in_table.end(), [&](const auto& x) {
            return x.name == unescaped_key;
         });
      if (maybe_column_metadata == columns_in_table.end()) {
         continue;
      }
      order_in_json_line.push_back(SniffedField{*maybe_column_metadata, std::string{raw_key}});
   }
   for (const auto& column_metadata : columns_in_table) {
      bool contained_in_sniffed_fields =
         std::find_if(
            order_in_json_line.begin(),
            order_in_json_line.end(),
            [&](const auto& sniffed_field) {
               return sniffed_field.column_identifier.name == column_metadata.name;
            }
         ) != order_in_json_line.end();
      if (!contained_in_sniffed_fields) {
         throw silo::append::AppendException(
            "the column '{}' is not contained in the following ndjson line: {}",
            column_metadata.name,
            contextForError(ndjson_line)
         );
      }
   }
   ndjson_line.rewind();
   return order_in_json_line;
}

void TablePartitionInserter::insert(
   simdjson::ondemand::document& ndjson_line,
   const std::vector<TablePartitionInserter::SniffedField>& field_order_hint
) const {
   EVOBENCH_SCOPE("TablePartitionInserter", "insert");
   auto columns = table_partition->columns.metadata;
   simdjson::ondemand::object object;
   auto error = ndjson_line.get_object().get(object);
   if (error) {
      throw silo::append::AppendException(
         "expect each ndjson line to be an object, got: {}", contextForError(ndjson_line)
      );
   }
   for (auto sniffed_field : field_order_hint) {
      auto column_value = findFieldWithFallbacks(object, sniffed_field);
      if (!column_value.has_value()) {
         throw silo::append::AppendException(
            "Key {} not found in the following ndjson line: {}",
            sniffed_field.column_identifier.name,
            contextForError(ndjson_line)
         );
      }
      auto success = table_partition->columns.addJsonValueToColumn(
         sniffed_field.column_identifier, column_value.value()
      );
      if (!success) {
         throw silo::append::AppendException(
            "{} in the following ndjson line: {}", success.error(), contextForError(ndjson_line)
         );
      }
   }
   table_partition->sequence_count++;
}

TablePartitionInserter::Commit TablePartitionInserter::commit() const {
   table_partition->finalize();
   table_partition->validate();
   return Commit{};
}

TablePartitionInserter TableInserter::openNewPartition() const {
   return TablePartitionInserter{table->addPartition()};
}

TableInserter::Commit TableInserter::commit() const {
   try {
      table->validate();
      return Commit{};
   } catch (const silo::schema::DuplicatePrimaryKeyException& exception) {
      throw silo::append::AppendException(exception.what());
   }
}

}  // namespace silo::append
