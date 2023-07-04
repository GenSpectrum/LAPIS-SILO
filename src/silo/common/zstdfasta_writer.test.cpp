#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/common/zstdfasta_reader.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/preprocessing/preprocessing_config.h"

TEST(ZstdFastaReader, writesCorrectFiles) {
   const std::filesystem::path file_path("./testBaseData/tmp/test.fasta");

   const std::string reference_genome = "ACGTACGTACGTACGT";
   const std::vector<std::pair<std::string, std::string>> values{
      {"Key1", "ACGTACGTACGTACGT"},
      {"Key2", "ACGTACGTACGTCCGT"},
      {"Key3", "ACGTACGTACGTACGT"},
      {"Key4", "CAGTTCGTACGTACGT"},
      {"Key5", "ACGTACGTACCTACGC"}};

   {
      silo::ZstdFastaWriter under_test(file_path, reference_genome);

      for (const auto& value : values) {
         under_test.write(value.first, value.second);
      }
   }

   {
      silo::ZstdFastaReader reader(file_path, reference_genome);

      std::string key;
      std::string genome;

      for (const auto& value : values) {
         EXPECT_TRUE(reader.next(key, genome));
         EXPECT_EQ(key, value.first);
         EXPECT_EQ(genome, value.second);
      }
      EXPECT_FALSE(reader.next(key, genome));
   }
}