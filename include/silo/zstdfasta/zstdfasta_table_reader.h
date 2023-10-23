#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace duckdb {
struct Connection;
struct MaterializedQueryResult;
struct DataChunk;
}  // namespace duckdb

namespace silo {
struct ZstdDecompressor;

class ZstdFastaTableReader {
  private:
   duckdb::Connection& connection;
   std::string_view table_name;
   std::unique_ptr<duckdb::MaterializedQueryResult> query_result;
   std::unique_ptr<duckdb::DataChunk> current_chunk;
   std::unique_ptr<silo::ZstdDecompressor> decompressor;
   size_t current_row;

   std::string DEBUG_dictionary;

   std::string genome_buffer;

   std::optional<std::string> nextKey();

  public:
   explicit ZstdFastaTableReader(
      duckdb::Connection& connection,
      std::string_view table_name,
      std::string_view compression_dict
   );

   std::optional<std::string> nextSkipGenome();

   std::optional<std::string> next(std::string& genome);

   std::optional<std::string> nextCompressed(std::string& compressed_genome);

   void reset();
};
}  // namespace silo
