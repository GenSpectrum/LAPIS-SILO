#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/config/preprocessing_config.h"
#include "silo/sequence_file_reader/fasta_format_exception.h"
#include "silo/sequence_file_reader/fasta_reader.h"

using silo::sequence_file_reader::FastaFormatException;
using silo::sequence_file_reader::FastaReader;

TEST(FastaReader, shouldReadFastaFile) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   FastaReader under_test(file_path);

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

TEST(FastaReader, shouldReadFastaFileWithBlankLines) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"blankLines.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   FastaReader under_test(file_path);

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

   FastaReader under_test(file_path);

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

   FastaReader under_test(file_path);

   EXPECT_THROW(under_test.nextEntry(), FastaFormatException);
}

TEST(FastaReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"missing_genome.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   FastaReader under_test(file_path);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, _, genome] = entry.value();
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(under_test.nextEntry(), FastaFormatException);
}

TEST(FastaReader, shouldReadFastaFileWithOffset) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"withOffset.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   FastaReader under_test(file_path);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, offset, genome] = entry.value();
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(offset, 10);
   EXPECT_EQ(genome, "ACGT");

   entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   key = entry.value().key;
   offset = entry.value().offset;
   genome = entry.value().sequence;
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(offset, 20);
   EXPECT_EQ(genome, "CGTA");

   entry = under_test.nextEntry();
   EXPECT_FALSE(entry.has_value());
}