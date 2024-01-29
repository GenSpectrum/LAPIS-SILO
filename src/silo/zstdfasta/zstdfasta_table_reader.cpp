#include "silo/zstdfasta/zstdfasta_table_reader.h"

#include <cstddef>
#include <fstream>
#include <stdexcept>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstd_decompressor.h"

silo::ZstdFastaTableReader::ZstdFastaTableReader(
   duckdb::Connection& connection,
   std::string_view table_name,
   std::string_view compression_dict,
   std::string_view sequence_column,
   std::string_view where_clause,
   std::string_view order_by_clause
)
    : connection(connection),
      table_name(table_name),
      sequence_column(sequence_column),
      where_clause(where_clause),
      order_by_clause(order_by_clause),
      decompressor(std::make_unique<ZstdDecompressor>(compression_dict)) {
   SPDLOG_TRACE("Initializing ZstdFastaTableReader for table {}", table_name);
   reset();
   SPDLOG_TRACE("Successfully initialized ZstdFastaTableReader for table {}", table_name);
}

std::optional<std::string> silo::ZstdFastaTableReader::nextKey() {
   if (!current_chunk) {
      return std::nullopt;
   }

   return current_chunk->GetValue(0, current_row).GetValue<std::string>();
}

std::optional<std::string> silo::ZstdFastaTableReader::nextSkipGenome() {
   auto key = nextKey();

   if (!key) {
      return std::nullopt;
   }

   advanceRow();
   return key;
}

std::optional<std::string> silo::ZstdFastaTableReader::nextCompressed(
   std::optional<std::string>& compressed_genome
) {
   auto key = nextKey();
   if (!key) {
      return std::nullopt;
   }

   const auto value = current_chunk->GetValue(1, current_row);
   if (value.IsNull()) {
      compressed_genome = std::nullopt;
   } else {
      compressed_genome = value.GetValueUnsafe<std::string>();
   }

   advanceRow();
   return key;
}

std::optional<std::string> silo::ZstdFastaTableReader::next(std::optional<std::string>& genome) {
   std::optional<std::string> compressed_buffer;
   auto key = nextCompressed(compressed_buffer);

   if (!key) {
      return std::nullopt;
   }

   if (compressed_buffer.has_value()) {
      genome = decompressor->decompress(compressed_buffer.value());
   } else {
      genome = std::nullopt;
   }

   return key;
}

std::string silo::ZstdFastaTableReader::getTableQuery() {
   return fmt::format(
      "SELECT key, {} FROM {} WHERE {} {}",
      sequence_column,
      table_name,
      where_clause,
      order_by_clause
   );
}

void silo::ZstdFastaTableReader::reset() {
   try {
      query_result = connection.Query(getTableQuery());
   } catch (const std::exception& e) {
      SPDLOG_ERROR("Error when executing SQL {}", e.what());
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "SQL for loading the results that the ZstdFastaTableReader reads:{}\n"
         "Resulting Error:\n{}",
         getTableQuery(),
         std::string(e.what())
      ));
   }
   if (query_result->HasError()) {
      SPDLOG_ERROR("Error when executing SQL " + query_result->GetError());
      throw silo::preprocessing::PreprocessingException(
         "Error when executing SQL " + query_result->GetError()
      );
   }
   current_chunk = query_result->Fetch();
   current_row = 0;

   while (current_chunk && current_chunk->size() == 0) {
      current_chunk = query_result->Fetch();
   }
}

void silo::ZstdFastaTableReader::advanceRow() {
   current_row++;
   if (current_row == current_chunk->size()) {
      current_row = 0;
      current_chunk = query_result->Fetch();
      while (current_chunk && current_chunk->size() == 0) {
         current_chunk = query_result->Fetch();
      }
   }
}

void silo::ZstdFastaTableReader::copyTableTo(std::string_view file_name) {
   query_result = connection.Query(fmt::format("COPY ({}) to '{}'", getTableQuery(), file_name));
   if (query_result->HasError()) {
      SPDLOG_ERROR("Error when executing SQL " + query_result->GetError());
      throw preprocessing::PreprocessingException("Error when SQL " + query_result->GetError());
   }
}

size_t silo::ZstdFastaTableReader::lineCount() {
   query_result = connection.Query(fmt::format("SELECT COUNT(*) FROM ({})", getTableQuery()));
   if (query_result->HasError()) {
      SPDLOG_ERROR("Error when executing SQL " + query_result->GetError());
      throw preprocessing::PreprocessingException("Error when SQL " + query_result->GetError());
   }
   const duckdb::Value count_value = query_result->GetValue(0, 0);
   const uint64_t line_count = duckdb::BigIntValue::Get(count_value);
   return static_cast<size_t>(line_count);
}
