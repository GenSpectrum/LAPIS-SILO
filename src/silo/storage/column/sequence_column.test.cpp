#include "silo/storage/column/sequence_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/storage/insertion_format_exception.h"

using silo::Nucleotide;
using silo::storage::InsertionFormatException;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::SequenceColumnPartition;

TEST(SequenceColumn, validErrorOnBadInsertionFormat_noTwoParts) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {Nucleotide::Symbol::A}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.appendInsertion("A"); },
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
      [&]() { under_test.appendInsertion("A:G"); },
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
         under_test.appendInsertion("0:EEEEE");
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
      [&]() { under_test.appendInsertion("0:0"); },
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
      [&]() { under_test.appendInsertion("0:"); },
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
      [&]() { under_test.appendInsertion(":A"); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected position "
                              "that is parsable as an integer, instead got: ':A'")
      )
   );
}

namespace {

void appendSequence(SequenceColumnPartition<Nucleotide>& column, std::string sequence) {
   auto& append_handle = column.appendNewSequenceRead();
   append_handle.offset = 0;
   append_handle.sequence = std::move(sequence);
   append_handle.is_valid = true;
}

}  // namespace

TEST(SequenceColumn, canFinalizeTwice) {
   SequenceColumnMetadata<Nucleotide> column_metadata{
      "test_column",
      {Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T}
   };
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   appendSequence(under_test, "AAGT");
   appendSequence(under_test, "AAGT");
   appendSequence(under_test, "AAGT");
   appendSequence(under_test, "ACGT");

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

   appendSequence(under_test, "ACGT");
   appendSequence(under_test, "ACGT");
   appendSequence(under_test, "ACGT");
   appendSequence(under_test, "ACGT");

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
