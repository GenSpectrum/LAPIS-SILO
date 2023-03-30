#include "silo/database.h"

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config.h"

silo::Database buildTestDatabase() {
   const silo::InputDirectory input_directory{"./testBaseData/"};
   const silo::OutputDirectory output_directory{"./build/"};
   const silo::MetadataFilename metadata_filename{"small_metadata_set.tsv"};
   const silo::SequenceFilename sequence_filename{"small_sequence_set.fasta"};
   auto config = silo::PreprocessingConfig(
      input_directory, output_directory, metadata_filename, sequence_filename
   );
   auto database = silo::Database(input_directory.directory);

   database.preprocessing(config);
   return database;
};

TEST(database_test, should_build_database_without_errors) {
   auto database{buildTestDatabase()};

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
}

TEST(database_info_test, should_return_correct_database_info) {
   auto database{buildTestDatabase()};

   const auto detailed_info = database.detailedDatabaseInfo();
   const auto simple_info = database.getDatabaseInfo();

   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::A), 9190510
   );
   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::GAP), 5779958
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_bitset_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_values_stored_in_run_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_bitset_containers,
      0
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_computed, 103449226
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_frozen, 55370197
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_array_containers,
      5859154
   );

   EXPECT_EQ(simple_info.total_size, 66458430);
   EXPECT_EQ(simple_info.n_bitmaps_size, 3552);
}