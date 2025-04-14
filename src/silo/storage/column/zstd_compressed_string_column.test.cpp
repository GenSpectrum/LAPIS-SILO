#include "silo/storage/column/zstd_compressed_string_column.h"

#include <gtest/gtest.h>

TEST(ZstdCompressedStringColumnPartition, insertValuesAndGetThemBack) {
   silo::storage::column::ZstdCompressedStringColumnMetadata column_metadata{"test_column", "ACGT"};
   silo::storage::column::ZstdCompressedStringColumnPartition under_test(&column_metadata);

   std::vector<std::optional<std::string>> values_to_add{
      "2020-01-01", "2023-01-05", "2021-12-03", "2025-01-01", std::nullopt, "2021-03-21", "asd"
   };
   for (const auto& value : values_to_add) {
      if (value.has_value()) {
         under_test.insert(value.value());
      } else {
         under_test.insertNull();
      }
   }

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      ASSERT_EQ(under_test.getDecompressed(value_idx), values_to_add.at(value_idx));
   }
}
