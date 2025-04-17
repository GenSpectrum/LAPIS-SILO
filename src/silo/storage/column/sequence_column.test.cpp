#include "silo/storage/column/sequence_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/storage/insertion_format_exception.h"

using silo::Nucleotide;
using silo::storage::InsertionFormatException;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::SequenceColumnPartition;

TEST(SequenceColumn, validErrorOnBadInsertionFormat_noTwoParts) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);

   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() { under_test.appendInsertion("A"); },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Failed to parse insertion due to invalid format. Expected two parts "
                              "(position and insertion value), instead got: 'A'")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_firstPartNotANumber) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {}};
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
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {}};
   SequenceColumnPartition<Nucleotide> under_test(&column_metadata);
   EXPECT_THAT(
      // NOLINTNEXTLINE(clang-diagnostic-error)
      [&]() {
         under_test.appendInsertion("0:EEEEE");
         under_test.finalize();
      },
      ThrowsMessage<InsertionFormatException>(
         ::testing::HasSubstr("Illegal nucleotide character 'E' in insertion: EEEEE")
      )
   );
}

TEST(SequenceColumn, validErrorOnBadInsertionFormat_firstPartEmpty) {
   SequenceColumnMetadata<Nucleotide> column_metadata{"test_column", {}};
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
