#include "silo/storage/column/sequence_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/append/append_exception.h"
#include "silo/storage/insertion_format_exception.h"

using silo::Nucleotide;
using silo::append::AppendException;
using silo::storage::InsertionFormatException;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::SequenceColumnPartition;

TEST(SequenceColumn, validErrorOnBadInsertionFormat_noTwoParts) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"A"}); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected two parts "
                              "(position and non-empty insertion value), instead got: 'A'")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_firstPartNotANumber) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"A:G"}); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected position "
                              "that is parsable as an integer, instead got: 'A:G'")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_secondPartIllegalSymbol) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() {
         under_test.append("A", 0, {"0:EEEEE"});
         under_test.finalize();
      },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Illegal nucleotide character 'E' in insertion: 0:EEEEE")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_secondPartIsANumber) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"0:0"}); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Illegal nucleotide character '0' in insertion: 0:0")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_secondPartEmpty) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"0:"}); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected two parts "
                              "(position and non-empty insertion value), instead got: '0:'")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_firstPartEmpty) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {":A"}); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected position "
                              "that is parsable as an integer, instead got: ':A'")
      )
   );
}

TEST(SequenceColumn, validErrorOnNegativeInsertionPosition) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"-5:G"}); },
      ThrowsMessage<InsertionFormatException>(::testing::HasSubstr("position must not be negative"))
   );
}

TEST(SequenceColumn, validErrorOnInsertionPositionOutOfRange) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.append("A", 0, {"100:G"}); },
      ThrowsMessage<AppendException>(::testing::HasSubstr(
         "the insertion position (100) is larger than the length of the reference sequence (1)"
      ))
   );
}

TEST(SequenceColumn, validInsertionAtPositionZero) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_NO_THROW(under_test.append("A", 0, {"0:G"}));
}

TEST(SequenceColumn, validInsertionAtPositionEqualToGenomeLength) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_NO_THROW(under_test.append("A", 0, {"1:G"}));
}

TEST(SequenceColumn, canFinalizeTwice) {
   SequenceColumnMetadata<Nucleotide> column_metadata{
      "test_column",
      {Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T}
   };
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   under_test.append("AAGT", 0, std::vector<std::string>{});
   under_test.append("AAGT", 0, std::vector<std::string>{});
   under_test.append("AAGT", 0, std::vector<std::string>{});
   under_test.append("ACGT", 0, std::vector<std::string>{});

   under_test.finalize();

   ASSERT_EQ(
      under_test.getLocalReference(),
      (std::vector<Nucleotide::Symbol>{
         Nucleotide::Symbol::A, Nucleotide::Symbol::A, Nucleotide::Symbol::G, Nucleotide::Symbol::T
      })
   );

   // This returns nothing, because symbol A is equal to reference -> nothing should be stored
   ASSERT_EQ(
      under_test.vertical_sequence_index.getMatchingContainersAsBitmap(1, {Nucleotide::Symbol::A}),
      (roaring::Roaring{})
   );

   under_test.append("ACGT", 0, std::vector<std::string>{});
   under_test.append("ACGT", 0, std::vector<std::string>{});
   under_test.append("ACGT", 0, std::vector<std::string>{});
   under_test.append("ACGT", 0, std::vector<std::string>{});

   under_test.finalize();

   ASSERT_EQ(
      under_test.vertical_sequence_index.getMatchingContainersAsBitmap(1, {Nucleotide::Symbol::A}),
      (roaring::Roaring{0, 1, 2})
   );

   ASSERT_EQ(
      under_test.getLocalReference(),
      (std::vector<Nucleotide::Symbol>{
         Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
      })
   );
}
