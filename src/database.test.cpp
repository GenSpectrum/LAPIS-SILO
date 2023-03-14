#include <silo/database.h>
#include <silo/preprocessing/preprocessing_config.h>
#include <string>
#include "gtest/gtest.h"

TEST(database_test, should_build_database_without_errors) {
   const std::string input_directory("./testBaseData/");
   const std::string output_directory("./build/");
   auto config = silo::PreprocessingConfig(
      input_directory, output_directory, "minimal_metadata_set.tsv", "minimal_sequence_set.fasta"
   );
   auto database = std::make_shared<silo::Database>(input_directory);

   EXPECT_NO_THROW(database->preprocessing(config));
   EXPECT_GT(static_cast<long int>(database->get_db_info().totalSize), 0);
   EXPECT_GT(static_cast<long int>(database->get_db_info().sequenceCount), 0);
}