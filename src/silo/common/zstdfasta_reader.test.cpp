#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/preprocessing/preprocessing_config.h"

TEST(ZstdFastaReader, shouldReadFastaFile) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.zstdfasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::ZstdFastaReader under_test(file_path, "ACGT");

   std::string key;
   std::string genome;
   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_FALSE(under_test.next(key, genome));
}

TEST(ZstdFastaReader, shouldReadFastaFileWithoutNewLineAtEnd) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"no_end_new_line.zstdfasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::ZstdFastaReader under_test(file_path, "ACGT");

   std::string key;
   std::string genome;
   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_FALSE(under_test.next(key, genome));
}

TEST(ZstdFastaReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"wrong_format.zstdfasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::ZstdFastaReader under_test(file_path, "ACGT");

   std::string key;
   std::string genome;
   EXPECT_THROW(under_test.next(key, genome), silo::FastaFormatException);
}

TEST(ZstdFastaReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"missing_genome.zstdfasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::ZstdFastaReader under_test(file_path, "ACGT");

   std::string key;
   std::string genome;
   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(under_test.next(key, genome), silo::FastaFormatException);
}