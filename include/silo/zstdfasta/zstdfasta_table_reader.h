#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "silo/zstdfasta/zstd_decompressor.h"

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
   std::string table_name;
   std::string sequence_column;
   std::string where_clause;
   std::string order_by_clause;
   std::unique_ptr<duckdb::MaterializedQueryResult> query_result;
   std::unique_ptr<duckdb::DataChunk> current_chunk;
   std::unique_ptr<ZstdDecompressor> decompressor;
   size_t current_row;

   std::optional<std::string> nextKey();

   std::string getTableQuery();

   void advanceRow();

  public:
   explicit ZstdFastaTableReader(
      duckdb::Connection& connection,
      std::string_view table_name,
      std::string_view compression_dict,
      std::string_view sequence_column,
      std::string_view where_clause,
      std::string_view order_by_clause
   );

   std::optional<std::string> nextSkipGenome();

   std::optional<std::string> next(std::optional<std::string>& genome);

   std::optional<std::string> nextCompressed(std::optional<std::string>& compressed_genome);

   void loadTable();

   void copyTableTo(std::string_view file_name);

   size_t lineCount();
};
}  // namespace silo
