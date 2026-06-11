#include "silo/storage/column/zstd_compressed_string_column.h"

#include <gtest/gtest.h>

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

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      ASSERT_EQ(under_test.getDecompressed(value_idx), values_to_add.at(value_idx));
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

// Each appendChunk starts a fresh, immutable chunk; row ids and the null bitmap continue across
// chunk boundaries.
TEST(ZstdCompressedStringColumn, valuesSpanningMultipleAppendedChunks) {
   silo::storage::column::ZstdCompressedStringColumnMetadata column_metadata{"test_column", "ACGT"};
   silo::storage::column::ZstdCompressedStringColumn under_test(&column_metadata);

   std::vector<std::optional<std::string>> all_values{
      "2020-01-01", "2023-01-05", std::nullopt, "2021-12-03", "2025-01-01", std::nullopt, "asd"
   };
   appendChunk(under_test, {all_values.begin(), all_values.begin() + 2});
   appendChunk(under_test, {all_values.begin() + 2, all_values.begin() + 6});
   appendChunk(under_test, {all_values.begin() + 6, all_values.end()});

   ASSERT_EQ(under_test.numValues(), all_values.size());
   for (size_t value_idx = 0; value_idx < all_values.size(); ++value_idx) {
      ASSERT_EQ(under_test.getDecompressed(value_idx), all_values.at(value_idx));
      ASSERT_EQ(under_test.isNull(value_idx), !all_values.at(value_idx).has_value());
   }
}
