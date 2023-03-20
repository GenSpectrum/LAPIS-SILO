#include "silo/database.h"

#include <gtest/gtest.h>
#include <string>

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

   EXPECT_GT(static_cast<long int>(database.getDatabaseInfo().totalSize), 0);
   EXPECT_EQ(static_cast<long int>(database.getDatabaseInfo().sequenceCount), 100);
}