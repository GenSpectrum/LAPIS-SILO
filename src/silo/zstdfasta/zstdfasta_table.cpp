#include "silo/zstdfasta/zstdfasta_table.h"

#include <fmt/format.h>
#include <duckdb.hpp>

#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"

namespace {
void initializeTable(duckdb::Connection& connection, std::string table_name) {
   auto return_value = connection.Query(fmt::format("DROP TABLE IF EXISTS {};", table_name));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->ToString());
   }
   return_value = connection.Query(fmt::format(
      "CREATE TABLE {} ("
      "    key STRING,"
      "    sequence BLOB"
      ");",
      table_name
   ));
   if (return_value->HasError()) {
      throw silo::preprocessing::PreprocessingException(return_value->ToString());
   }
}
}  // namespace

void silo::ZstdFastaTable::generate(
   duckdb::Connection& connection,
   const std::string& table_name,
   silo::ZstdFastaReader& file_reader
) {
   initializeTable(connection, table_name);
   std::optional<std::string> key;
   std::string compressed_genome;
   duckdb::Appender appender(connection, table_name);
   while (true) {
      key = file_reader.nextCompressed(compressed_genome);
      if (key == std::nullopt) {
         break;
      }
      const size_t compressed_size = compressed_genome.size();
      const auto* compressed_data =
         reinterpret_cast<const unsigned char*>(compressed_genome.data());
      const duckdb::string_t key_value = key.value();
      appender.BeginRow();
      appender.Append(key_value);
      appender.Append(duckdb::Value::BLOB(compressed_data, compressed_size));
      appender.EndRow();
   }
   appender.Close();
}

void silo::ZstdFastaTable::generate(
   duckdb::Connection& connection,
   const std::string& table_name,
   silo::FastaReader& file_reader,
   std::string_view reference_sequence
) {
   initializeTable(connection, table_name);
   std::optional<std::string> key;
   std::string uncompressed_genome;
   silo::ZstdCompressor compressor(reference_sequence);
   std::string compressed_genome;
   compressed_genome.resize(compressor.getSizeBound());
   duckdb::Appender appender(connection, table_name);
   while (true) {
      key = file_reader.next(uncompressed_genome);
      if (key == std::nullopt) {
         break;
      }
      const size_t compressed_size = compressor.compress(uncompressed_genome, compressed_genome);
      const auto* compressed_data =
         reinterpret_cast<const unsigned char*>(compressed_genome.data());
      const duckdb::string_t key_value = key.value();
      appender.BeginRow();
      appender.Append(key_value);
      appender.Append(duckdb::Value::BLOB(compressed_data, compressed_size));
      appender.EndRow();
   }
   appender.Close();
}
