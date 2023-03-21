#include "silo/database.h"

#include <gtest/gtest.h>

#include "silo/common/genome_symbols.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config.h"

TEST(database_test, should_build_database_without_errors) {
   const silo::InputDirectory input_directory{"./testBaseData/"};
   const silo::OutputDirectory output_directory{"./build/"};
   const silo::MetadataFilename metadata_filename{"small_metadata_set.tsv"};
   const silo::SequenceFilename sequence_filename{"small_sequence_set.fasta"};
   auto config = silo::PreprocessingConfig(
      input_directory, output_directory, metadata_filename, sequence_filename
   );
   auto database = silo::Database(input_directory.directory);

   database.preprocessing(config);

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(static_cast<long int>(simple_database_info.totalSize), 0);
   EXPECT_EQ(static_cast<long int>(simple_database_info.sequenceCount), 100);
}

TEST(database_info_test, should_return_correct_database_info) {
   const silo::InputDirectory input_directory{"./testBaseData/"};
   const silo::OutputDirectory output_directory{"./build/"};
   const silo::MetadataFilename metadata_filename{"small_metadata_set.tsv"};
   const silo::SequenceFilename sequence_filename{"small_sequence_set.fasta"};
   auto config = silo::PreprocessingConfig(
      input_directory, output_directory, metadata_filename, sequence_filename
   );
   auto database = silo::Database(input_directory.directory);

   database.preprocessing(config);

   const auto detailed_info = database.detailedDatabaseInfo();
   const auto simple_info = database.getDatabaseInfo();

   EXPECT_EQ(detailed_info.bitmapSizePerSymbol.sizeInBytes.at(silo::GENOME_SYMBOL::A), 100);
   EXPECT_EQ(simple_info.totalSize, 66458430);
   EXPECT_EQ(simple_info.nBitmapsSize, 3552);
}