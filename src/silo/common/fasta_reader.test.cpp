#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/preprocessing_config.h"

TEST(FastaReader, shouldReadFastaFile) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

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

TEST(FastaReader, shouldReadFastaFileWithoutNewLineAtEnd) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"no_end_new_line.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::string key;
   std::string genome;
   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_FALSE(under_test.next(key, genome));
}

TEST(FastaReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"wrong_format.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::string key;
   std::string genome;
   EXPECT_THROW(under_test.next(key, genome), silo::FastaFormatException);
}

TEST(FastaReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::filesystem::path input_directory{"./testBaseData/fastaFiles/"};
   const std::string sequence_filename{"missing_genome.fasta"};

   std::filesystem::path file_path = input_directory;
   file_path += sequence_filename;

   silo::FastaReader under_test(file_path);

   std::string key;
   std::string genome;
   EXPECT_TRUE(under_test.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(under_test.next(key, genome), silo::FastaFormatException);
}