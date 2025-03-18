#include "silo/append/database_inserter.h"
#include "silo/schema/duplicate_primary_key_exception.h"

namespace silo::append {

void TablePartitionInserter::insert(const nlohmann::json& ndjson_line) const {
   for (auto& column_metadata : table_partition->columns.metadata) {
      try {
         table_partition->columns.addJsonValueToColumn(column_metadata, ndjson_line);
      } catch (const nlohmann::json::type_error& error) {
         throw silo::append::AppendException(
            "The following line does not conform to SILO's json specification when adding "
            "to database column {}: '{}'\n"
            "We got a json type_error: {}",
            column_metadata.name,
            ndjson_line.dump(),
            error.what()
         );
      } catch (const nlohmann::json::out_of_range& error) {
         throw silo::append::AppendException(
            "The following line does not conform to SILO's json specification when adding "
            "to database column {}: '{}'\n"
            "We got a json out_of_range error, indicating that an expected field was not present: "
            "{}",
            column_metadata.name,
            ndjson_line.dump(),
            error.what()
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
