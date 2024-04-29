#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/config/preprocessing_config.h"
#include "silo/zstdfasta/zstd_decompressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"
#include "silo/zstdfasta/zstdfasta_writer.h"

TEST(ZstdFastaWriter, writesCorrectFiles) {
   const std::filesystem::path file_path("./testBaseData/tmp/test.fasta");

   const std::string reference_genome = "ACGTACGTACGTACGT";
   const std::vector<std::pair<std::string, std::string>> values{
      {"Key1", "ACGTACGTACGTACGT"},
      {"Key2", "ACGTACGTACGTCCGT"},
      {"Key3", "ACGTACGTACGTACGT"},
      {"Key4", "CAGTTCGTACGTACGT"},
      {"Key5", "ACGTACGTACCTACGC"}
   };

   {
      silo::ZstdFastaWriter under_test(file_path, reference_genome);

      for (const auto& value : values) {
         under_test.write(value.first, value.second);
      }
   }

   {
      silo::ZstdFastaReader reader(file_path, reference_genome);

      std::optional<std::string> key;
      std::string genome;

      for (const auto& value : values) {
         key = reader.next(genome);
         EXPECT_TRUE(key != std::nullopt);
         EXPECT_EQ(key, value.first);
         EXPECT_EQ(genome, value.second);
      }
      key = reader.next(genome);
      EXPECT_FALSE(key != std::nullopt);
   }
}
