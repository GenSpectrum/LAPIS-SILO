#include "silo/zstd/zstd_table.h"

#include <fmt/format.h>
#include <silo/sequence_file_reader/sequence_file_reader.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstd/zstd_compressor.h"

namespace {
void initializeTable(duckdb::Connection& connection, std::string table_name) {
   auto return_value = connection.Query(fmt::format("DROP TABLE IF EXISTS {};", table_name));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->GetError());
   }
   return_value = connection.Query(fmt::format(
      "CREATE TABLE {} ("
      "    key STRING,"
      "    read STRUCT(\"offset\" UINTEGER, sequence BLOB)"
      ");",
      table_name
   ));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->GetError());
   }
}
}  // namespace

namespace silo {

using silo::sequence_file_reader::SequenceFileReader;

ZstdTable ZstdTable::generate(
   duckdb::Connection& connection,
   const std::string& table_name,
   SequenceFileReader& file_reader,
   std::string_view reference_sequence
) {
   initializeTable(connection, table_name);
   std::optional<silo::SequenceFileReader::ReadSequence> entry;
   ZstdCompressor compressor(std::make_shared<ZstdCDictionary>(reference_sequence, 2));
   duckdb::Appender appender(connection, table_name);
   while (true) {
      entry = file_reader.nextEntry();
      if (!entry) {
         break;
      }
      const std::string_view compressed = compressor.compress(entry->sequence);
      const auto* compressed_data = reinterpret_cast<const unsigned char*>(compressed.data());
      const duckdb::string_t key_value = entry->key;
      appender.BeginRow();
      appender.Append(key_value);
      appender.Append(duckdb::Value::STRUCT(
         {{"\"offset\"", duckdb::Value::UINTEGER(entry.value().offset)},
          {"sequence", duckdb::Value::BLOB(compressed_data, compressed.size())}}
      ));
      appender.EndRow();
   }
   appender.Close();
   return {connection, table_name};
}

}  // namespace silo