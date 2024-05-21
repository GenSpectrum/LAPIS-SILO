#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/file_reader/fasta_format_exception.h"
#include "silo/file_reader/fasta_reader.h"
#include "silo/config/preprocessing_config.h"

TEST(FastaReader, shouldReadFastaFile) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, _, genome] = entry.value();
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   key = entry.value().key;
   genome = entry.value().sequence;
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   entry = under_test.nextEntry();
   EXPECT_FALSE(entry.has_value());
}

TEST(FastaReader, shouldReadFastaFileWithoutNewLineAtEnd) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"no_end_new_line.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, _, genome] = entry.value();
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   entry = under_test.nextEntry();
   EXPECT_FALSE(entry.has_value());
}

TEST(FastaReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"wrong_format.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   EXPECT_THROW(under_test.nextEntry(), silo::FastaFormatException);
}

TEST(FastaReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"missing_genome.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, _, genome] = entry.value();
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(under_test.nextEntry(), silo::FastaFormatException);
}