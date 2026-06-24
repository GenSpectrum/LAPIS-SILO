#include "silo/storage/column/zstd_compressed_string_column.h"

#include <gtest/gtest.h>

#include "silo/storage/column/row_id.h"

using silo::storage::column::RowId;

TEST(ZstdCompressedStringColumn, insertValuesAndGetThemBack) {
   silo::storage::column::ZstdCompressedStringColumnMetadata column_metadata{"test_column", "ACGT"};
   silo::storage::column::ZstdCompressedStringColumn under_test(&column_metadata);

   std::vector<std::optional<std::string>> values_to_add{
      "2020-01-01", "2023-01-05", "2021-12-03", "2025-01-01", std::nullopt, "2021-03-21", "asd"
   };
   silo::storage::column::ZstdCompressedStringColumn::Builder builder;
   for (const auto& value : values_to_add) {
      if (value.has_value()) {
         builder.insert(value.value());
      } else {
         builder.insertNull();
      }
   }
   SILO_ASSERT(under_test.appendChunk(builder.finalize()).has_value());

   ASSERT_EQ(under_test.numChunks(), 1);
   ASSERT_EQ(under_test.chunkSize(0), values_to_add.size());

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      ASSERT_EQ(under_test.getDecompressed(RowId(0, value_idx)), values_to_add.at(value_idx));
   }
}

namespace {
void appendChunk(
   silo::storage::column::ZstdCompressedStringColumn& column,
   const std::vector<std::optional<std::string>>& values
) {
   silo::storage::column::ZstdCompressedStringColumn::Builder builder;
   for (const auto& value : values) {
      if (value.has_value()) {
         builder.insert(value.value());
      } else {
         builder.insertNull();
      }
   }
   SILO_ASSERT(column.appendChunk(builder.finalize()).has_value());
}
}  // namespace

// Each appendChunk starts a fresh, immutable chunk whose global row ids begin at a fresh
// 2^16-aligned offset (chunk k starts at k << 16); the null bitmap stores those aligned row ids.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ZstdCompressedStringColumn, valuesSpanningMultipleAppendedChunks) {
   using silo::storage::column::RowId;
   silo::storage::column::ZstdCompressedStringColumnMetadata column_metadata{"test_column", "ACGT"};
   silo::storage::column::ZstdCompressedStringColumn under_test(&column_metadata);

   const std::vector<std::vector<std::optional<std::string>>> chunks{
      {"2020-01-01", "2023-01-05"},
      {std::nullopt, "2021-12-03", "2025-01-01", std::nullopt},
      {"asd"}
   };
   const auto num_chunks = static_cast<uint16_t>(chunks.size());
   for (const auto& chunk : chunks) {
      appendChunk(under_test, chunk);
   }

   ASSERT_EQ(under_test.numChunks(), chunks.size());

   for (uint16_t chunk_id = 0; chunk_id < num_chunks; ++chunk_id) {
      ASSERT_EQ(under_test.chunkSize(chunk_id), chunks.at(chunk_id).size());
   }

   for (uint16_t chunk_id = 0; chunk_id < num_chunks; ++chunk_id) {
      const auto& chunk = chunks.at(chunk_id);
      for (size_t row_in_chunk = 0; row_in_chunk < chunk.size(); ++row_in_chunk) {
         const auto row_id = RowId(chunk_id, static_cast<uint16_t>(row_in_chunk));
         ASSERT_EQ(under_test.getDecompressed(row_id), chunk.at(row_in_chunk));
         ASSERT_EQ(under_test.isNull(row_id), !chunk.at(row_in_chunk).has_value());
      }
   }
}
