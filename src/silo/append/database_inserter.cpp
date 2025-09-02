#include "silo/append/database_inserter.h"

#include <algorithm>

#include "evobench/evobench.hpp"
#include "silo/common/error.h"
#include "silo/schema/duplicate_primary_key_exception.h"

namespace silo::append {

namespace {

std::expected<simdjson::ondemand::value, std::string> findFieldManual(
   simdjson::ondemand::object& object,
   const silo::schema::ColumnIdentifier& column_identifier
) {
   object.reset();
   for (auto maybe_field : object) {
      ASSIGN_OR_RAISE_SIMDJSON(
         simdjson::ondemand::field,
         field,
         std::move(maybe_field),
         "Could not parse field from object"
      );
      ASSIGN_OR_RAISE_SIMDJSON(
         std::string_view,
         unescaped_key,
         field.unescaped_key(),
         "Could not unescape the key {}. Probably a utf-8 error",
         field.escaped_key()
      );
      if (unescaped_key == column_identifier.name) {
         static std::once_flag warn_once;
         std::call_once(warn_once, [&]() {
            SPDLOG_WARN(
               "The key '{}' which requires unescaping does not use the same unescaping in the "
               "current line ('{}') as in the first line of the ndjson file. This leads to worse "
               "performance during parsing",
               unescaped_key,
               field.escaped_key()
            );
         });
         return field.value();
      }
   }
   return std::unexpected(
      fmt::format("Did not find the field '{}' in the given json", column_identifier.name)
   );
}

std::expected<simdjson::ondemand::value, std::string> findFieldWithFallbacks(
   simdjson::ondemand::object& object,
   const TablePartitionInserter::SniffedField& sniffed_field
) {
   simdjson::ondemand::value column_value;
   auto error = object.find_field(sniffed_field.escaped_key).get(column_value);
   if (!error) {
      return column_value;
   }
   error = object.find_field_unordered(sniffed_field.escaped_key).get(column_value);
   if (!error) {
      static std::once_flag warn_once;
      std::call_once(warn_once, [&]() {
         SPDLOG_WARN(
            "The key '{}' was ordered differently in the current line than in the first line of "
            "the file. "
            "There might be an error in the data generation. "
            "The performance is better if the fields are ordered regularly.",
            sniffed_field.escaped_key
         );
      });
      return column_value;
   }
   // Even for fields that require escaping, we get the escaped value from the sniffed field, so
   // this should never occur in well-formatted ndjson files. Print a warning in findFieldManual
   return findFieldManual(object, sniffed_field.column_identifier);
}

std::expected<simdjson::ondemand::object, std::string> iterateToObject(
   simdjson::ondemand::document_reference ndjson_line
) {
   simdjson::ondemand::object object;
   auto error = ndjson_line.get_object().get(object);
   if (error) {
      if (error == simdjson::INCOMPLETE_ARRAY_OR_OBJECT) {
         return std::unexpected(
            "the ndjson line does not contain valid json (incomplete object or array)"
         );
      } else if (error == simdjson::INCORRECT_TYPE) {
         simdjson::ondemand::json_type type;
         error = ndjson_line.type().get(type);
         if (!error) {
            // type does not offer a toString-like method, but only this stream-method
            std::stringstream error_message;
            error_message << "expect each ndjson line to be an object, got type '" << type << "'";
            return std::unexpected(error_message.str());
         }
      }
      return std::unexpected(
         fmt::format("unexpected error {} while parsing line", simdjson::error_message(error))
      );
   }
   return object;
}

}  // namespace

std::expected<std::vector<TablePartitionInserter::SniffedField>, std::string>
TablePartitionInserter::sniffFieldOrder(simdjson::ondemand::document_reference ndjson_line) const {
   std::vector<SniffedField> order_in_json_line;
   auto columns_in_table = table_partition->columns.metadata;
   ASSIGN_OR_RAISE(auto object, iterateToObject(ndjson_line));
   for (auto maybe_field : object) {
      ASSIGN_OR_RAISE_SIMDJSON(
         simdjson::ondemand::field,
         field,
         std::move(maybe_field),
         "error '{}', while parsing a field of the json object"
      );

      std::string_view raw_key_sv = field.escaped_key();

      ASSIGN_OR_RAISE_SIMDJSON(
         std::string_view,
         unescaped_key,
         field.unescaped_key(),
         "error '{1}', when unescaping the key '{0}'",
         raw_key_sv
      );

      auto maybe_column_metadata =
         std::find_if(columns_in_table.begin(), columns_in_table.end(), [&](const auto& x) {
            return x.name == unescaped_key;
         });
      if (maybe_column_metadata == columns_in_table.end()) {
         SPDLOG_WARN(
            "The field '{}' which is contained in the input json file is not in the database. "
            "Ignoring field.",
            unescaped_key
         );
         continue;
      }
      order_in_json_line.push_back(SniffedField{*maybe_column_metadata, std::string{raw_key_sv}});
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
         return std::unexpected{
            fmt::format("the column '{}' is not contained in the object", column_metadata.name)
         };
      }
   }
   ndjson_line.rewind();
   return order_in_json_line;
}

std::expected<void, std::string> TablePartitionInserter::insert(
   simdjson::ondemand::document_reference ndjson_line,
   const std::vector<TablePartitionInserter::SniffedField>& field_order_hint
) const {
   EVOBENCH_SCOPE_EVERY(20, "TablePartitionInserter", "insert");
   ASSIGN_OR_RAISE(auto object, iterateToObject(ndjson_line));
   for (auto sniffed_field : field_order_hint) {
      ASSIGN_OR_RAISE(auto column_value, findFieldWithFallbacks(object, sniffed_field));
      auto success_or_error = table_partition->columns.addJsonValueToColumn(
         sniffed_field.column_identifier, column_value
      );
      if (!success_or_error.has_value()) {
         return success_or_error;
      }
   }
   table_partition->sequence_count++;
   return {};
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

TablePartitionInserter::Commit appendDataToTablePartition(
   TablePartitionInserter partition_inserter,
   NdjsonLineReader& input_data
) {
   EVOBENCH_SCOPE("TablePartitionInserter", "appendDataToTablePartition");
   size_t line_count = 0;

   bool first_line = true;

   std::vector<TablePartitionInserter::SniffedField> sniffed_field_order;
   for (auto [json_obj_or_error, raw_line] : input_data) {
      simdjson::ondemand::document_reference ndjson_line;
      auto error = json_obj_or_error.get(ndjson_line);
      if (error) {
         throw AppendException(
            "get error '{}' when parsing the current line: {}",
            simdjson::error_message(error),
            raw_line
         );
      }

      if (first_line) {
         auto sniffed_field_order_or_error = partition_inserter.sniffFieldOrder(ndjson_line);
         if (!sniffed_field_order_or_error.has_value()) {
            throw AppendException{
               "{} - current line: {}", sniffed_field_order_or_error.error(), raw_line
            };
         }
         sniffed_field_order = sniffed_field_order_or_error.value();
         first_line = false;
      }

      auto maybe_error = partition_inserter.insert(ndjson_line, sniffed_field_order);
      if (!maybe_error.has_value()) {
         throw AppendException{"{} - current line: {}", maybe_error.error(), raw_line};
      }

      line_count++;
      if (line_count % 10000 == 0) {
         SPDLOG_INFO("Processed {} json objects from the input file", line_count);
      }
   }

   return partition_inserter.commit();
}

TableInserter::Commit appendDataToTable(
   std::shared_ptr<silo::storage::Table> table,
   NdjsonLineReader& input_data
) {
   TableInserter table_inserter(table);

   // TODO(#738) make partition configurable
   auto table_partition = table_inserter.openNewPartition();

   appendDataToTablePartition(table_partition, input_data);

   return table_inserter.commit();
}

void appendDataToDatabase(Database& database, NdjsonLineReader& input_data) {
   appendDataToTable(database.table, input_data);
   database.updateDataVersion();
}

}  // namespace silo::append
