#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/fasta_reader.h"
#include "silo/preprocessing/preprocessing_config.h"

TEST(FastaReader, shouldReadFastaFile) {
   const silo::InputDirectory input_directory{"./testBaseData/fastaFiles/"};
   const silo::SequenceFilename sequence_filename{"test.fasta"};
   const std::filesystem::path file_path =
      silo::createPath(input_directory.directory, sequence_filename.filename);
   silo::FastaReader in_file(file_path);

   std::string key;
   std::string genome;
   EXPECT_TRUE(in_file.next(key, genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_TRUE(in_file.next(key, genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_FALSE(in_file.next(key, genome));
}

TEST(FastaReader, shouldReadFastaFileWithoutNewLineAtEnd) {
   const silo::InputDirectory input_directory{"./testBaseData/fastaFiles/"};
   const silo::SequenceFilename sequence_filename{"no_end_new_line.fasta"};
   const std::filesystem::path file_path =
      silo::createPath(input_directory.directory, sequence_filename.filename);
   silo::FastaReader in_file(file_path);

   std::string key;
   std::string genome;
   EXPECT_TRUE(in_file.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_FALSE(in_file.next(key, genome));
}

TEST(FastaReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const silo::InputDirectory input_directory{"./testBaseData/fastaFiles/"};
   const silo::SequenceFilename sequence_filename{"wrong_format.fasta"};
   const std::filesystem::path file_path =
      silo::createPath(input_directory.directory, sequence_filename.filename);
   silo::FastaReader wrong_in_file(file_path);

   std::string key;
   std::string genome;
   EXPECT_THROW(wrong_in_file.next(key, genome), silo::FastaFormatException);
}

TEST(FastaReader, givenDataInWrongFormatThenShouldThrowAnException2) {
   const silo::InputDirectory input_directory{"./testBaseData/fastaFiles/"};
   const silo::SequenceFilename sequence_filename{"missing_genome.fasta"};
   const std::filesystem::path file_path =
      silo::createPath(input_directory.directory, sequence_filename.filename);
   silo::FastaReader wrong_in_file(file_path);

   std::string key;
   std::string genome;
   EXPECT_TRUE(wrong_in_file.next(key, genome));
   EXPECT_EQ(key, "Key");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_THROW(wrong_in_file.next(key, genome), silo::FastaFormatException);
}