#include "silo/zstdfasta/zstdfasta_table_reader.h"

#include <cstddef>
#include <fstream>
#include <stdexcept>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>
#include <duckdb/common/types/blob.hpp>

#include "silo/common/fasta_format_exception.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstd_decompressor.h"

silo::ZstdFastaTableReader::ZstdFastaTableReader(
   duckdb::Connection& connection,
   std::string_view table_name,
   std::string_view compression_dict,
   std::string_view where_clause
)
    : connection(connection),
      table_name(table_name),
      where_clause(where_clause),
      decompressor(std::make_unique<ZstdDecompressor>(compression_dict)),
      DEBUG_dictionary(compression_dict) {
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

   current_row++;
   while (current_chunk && current_row == current_chunk->size()) {
      current_chunk = query_result->Fetch();
   }

   return key;
}

std::optional<std::string> silo::ZstdFastaTableReader::nextCompressed(std::string& compressed_genome
) {
   auto key = nextKey();
   if (!key) {
      return std::nullopt;
   }

   SPDLOG_TRACE("Evaluating GetValueUnsafe command");
   compressed_genome = current_chunk->GetValue(1, current_row).GetValueUnsafe<std::string>();
   SPDLOG_TRACE("Successfully evaluated GetValueUnsafe command");

   current_row++;
   while (current_chunk && current_row == current_chunk->size()) {
      current_chunk = query_result->Fetch();
   }

   return key;
}

std::optional<std::string> silo::ZstdFastaTableReader::next(std::string& genome) {
   std::string compressed_buffer;
   auto key = nextCompressed(compressed_buffer);

   if (!key) {
      return std::nullopt;
   }

   decompressor->decompress(compressed_buffer, genome_buffer);
   genome = genome_buffer;  // TODO is the copy necessary?

   return key;
}

void silo::ZstdFastaTableReader::reset() {
   // TODO assert that it is and define what is a 'zstdfastatable'
   query_result = connection.Query(
      fmt::format("SELECT key, sequence FROM {} WHERE {}", table_name, where_clause)
   );
   if (query_result->HasError()) {
      SPDLOG_ERROR("Error when executing SQL " + query_result->GetError());
      throw silo::PreprocessingException("Error when SQL " + query_result->GetError());
   }
   current_chunk = query_result->Fetch();
   current_row = 0;

   while (current_chunk && current_row == current_chunk->size()) {
      current_chunk = query_result->Fetch();
   }
}
