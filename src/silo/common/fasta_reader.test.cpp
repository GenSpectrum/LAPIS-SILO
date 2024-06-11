#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/fasta_reader.h"
#include "silo/config/preprocessing_config.h"

TEST(FastaReader, shouldReadFastaFile) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::optional<std::string> key;
   std::string genome;
   key = under_test.next(genome);
   EXPECT_TRUE(key != std::nullopt);
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   key = under_test.next(genome);
   EXPECT_TRUE(key != std::nullopt);
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   key = under_test.next(genome);
   EXPECT_FALSE(key != std::nullopt);
}

TEST(FastaReader, shouldReadFastaFileWithoutNewLineAtEnd) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"no_end_new_line.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::optional<std::string> key;
   std::string genome;

   key = under_test.next(genome);
   EXPECT_TRUE(key != std::nullopt);
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   key = under_test.next(genome);
   EXPECT_FALSE(key != std::nullopt);
}

TEST(FastaReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"wrong_format.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::string genome;
   EXPECT_THROW(under_test.next(genome), silo::FastaFormatException);
}

TEST(FastaReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"missing_genome.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::optional<std::string> key;
   std::string genome;

   key = under_test.next(genome);
   EXPECT_TRUE(key != std::nullopt);
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(under_test.next(genome), silo::FastaFormatException);
}