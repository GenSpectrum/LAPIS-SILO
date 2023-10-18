#include "silo/zstdfasta/zstdfasta_table.h"

#include <fmt/format.h>
#include <duckdb.hpp>

#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"

namespace {
void initializeTable(duckdb::Connection& connection, std::string table_name) {
   auto prepared_statement = connection.Prepare(
      "CREATE TABLE ? ("
      "    key STRING,"
      "    compressedGenome BLOB"
      ");"
   );
   auto return_value = prepared_statement->Execute(table_name);
   if (return_value->HasError()) {
      throw silo::PreprocessingException(return_value->ToString());
   }
}
}  // namespace

void silo::ZstdFastaTable::generate(
   duckdb::Connection& connection,
   std::string table_name,
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
      appender.AppendRow(key.value(), duckdb::Value::BLOB(compressed_genome));
   }
   appender.Close();
}

void silo::ZstdFastaTable::generate(
   duckdb::Connection& connection,
   std::string table_name,
   silo::FastaReader& file_reader,
   std::string_view reference_genome
) {
   initializeTable(connection, table_name);
   std::optional<std::string> key;
   std::string uncompressed_genome;
   silo::ZstdCompressor compressor(reference_genome);
   std::string compressed_genome;
   compressed_genome.resize(compressor.getSizeBound());
   duckdb::Appender appender(connection, table_name);
   while (true) {
      key = file_reader.next(uncompressed_genome);
      if (key == std::nullopt) {
         break;
      }
      size_t compressed_size = compressor.compress(uncompressed_genome, compressed_genome);
      auto compressed_data = reinterpret_cast<const unsigned char*>(compressed_genome.data());
      appender.AppendRow(key.value(), duckdb::Value::BLOB(compressed_data, compressed_size));
   }
   appender.Close();
}
