#include "silo/zstdfasta/zstdfasta_table_reader.h"

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
   std::string_view where_clause,
   std::string_view order_by_clause
)
    : connection(connection),
      table_name(table_name),
      where_clause(where_clause),
      order_by_clause(order_by_clause),
      decompressor(std::make_unique<ZstdDecompressor>(compression_dict)) {
   SPDLOG_TRACE("Initializing ZstdFastaTableReader for table {}", table_name);
   genome_buffer.resize(compression_dict.size());
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
      decompressor->decompress(compressed_buffer.value(), genome_buffer);
      genome = genome_buffer;
   } else {
      genome = std::nullopt;
   }

   return key;
}

void silo::ZstdFastaTableReader::reset() {
   try {
      query_result = connection.Query(fmt::format(
         "SELECT key, sequence FROM {} WHERE {} {}", table_name, where_clause, order_by_clause
      ));
   } catch (const std::exception& e) {
      SPDLOG_ERROR("Error when executing SQL {}", e.what());
      throw silo::preprocessing::PreprocessingException(
         "Error when executing SQL " + std::string(e.what())
      );
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
