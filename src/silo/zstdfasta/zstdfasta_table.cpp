#include "silo/zstdfasta/zstdfasta_table.h"

#include <fmt/format.h>
#include <duckdb.hpp>

#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

namespace {
void initializeTable(duckdb::Connection& connection, std::string table_name) {
   auto return_value = connection.Query(fmt::format("DROP TABLE IF EXISTS {};", table_name));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->GetError());
   }
   return_value = connection.Query(fmt::format(
      "CREATE TABLE {} ("
      "    key STRING,"
      "    sequence BLOB"
      ");",
      table_name
   ));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->GetError());
   }
}
}  // namespace

namespace silo {

ZstdFastaTable::ZstdFastaTable(
   duckdb::Connection& connection,
   std::string table_name,
   std::string_view compression_dict
)
    : connection(connection),
      table_name(std::move(table_name)),
      compression_dict(compression_dict) {}

ZstdFastaTable ZstdFastaTable::generate(
   duckdb::Connection& connection,
   const std::string& table_name,
   ZstdFastaReader& file_reader,
   std::string_view reference_sequence
) {
   initializeTable(connection, table_name);
   std::optional<std::string> key;
   std::string compressed;
   duckdb::Appender appender(connection, table_name);
   while (true) {
      key = file_reader.nextCompressed(compressed);
      if (key == std::nullopt) {
         break;
      }
      const size_t compressed_size = compressed.size();
      const auto* compressed_data = reinterpret_cast<const unsigned char*>(compressed.data());
      const duckdb::string_t key_value = key.value();
      appender.BeginRow();
      appender.Append(key_value);
      appender.Append(duckdb::Value::BLOB(compressed_data, compressed_size));
      appender.EndRow();
   }
   appender.Close();
   return {connection, table_name, reference_sequence};
}

ZstdFastaTable ZstdFastaTable::generate(
   duckdb::Connection& connection,
   const std::string& table_name,
   FastaReader& file_reader,
   std::string_view reference_sequence
) {
   initializeTable(connection, table_name);
   std::optional<silo::FastaReader::SequenceIdentifier> identifier;
   std::string uncompressed;
   ZstdCompressor compressor(std::make_shared<ZstdCDictionary>(reference_sequence, 2));
   duckdb::Appender appender(connection, table_name);
   while (true) {
      identifier = file_reader.next(uncompressed);
      if (identifier == std::nullopt) {
         break;
      }
      const std::string_view compressed = compressor.compress(uncompressed);
      const auto* compressed_data = reinterpret_cast<const unsigned char*>(compressed.data());
      const duckdb::string_t key_value = identifier->key;
      appender.BeginRow();
      appender.Append(key_value);
      appender.Append(duckdb::Value::BLOB(compressed_data, compressed.size()));
      appender.EndRow();
   }
   appender.Close();
   return {connection, table_name, reference_sequence};
}

ZstdFastaTableReader ZstdFastaTable::getReader(
   std::string_view where_clause,
   std::string_view order_by_clause
) {
   return ZstdFastaTableReader(
      connection, table_name, compression_dict, "sequence", where_clause, order_by_clause
   );
}

}  // namespace silo