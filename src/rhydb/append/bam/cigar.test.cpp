#include "rhydb/append/bam/cigar.h"

#include <gtest/gtest.h>

using rhydb::append::bam::CigarElement;
using rhydb::append::bam::CigarOp;
using rhydb::append::bam::projectReadOntoReference;

namespace {

CigarElement op(uint32_t length, CigarOp operation) {
   return CigarElement{.length = length, .op = operation};
}

}  // namespace

TEST(Cigar, plainMatchCopiesReadAtOffset) {
   auto result = projectReadOntoReference(10, "ACGT", {op(4, CigarOp::MATCH)});
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 10);
   EXPECT_EQ(result->aligned_sequence, "ACGT");
   EXPECT_TRUE(result->insertions.empty());
}

TEST(Cigar, deletionBecomesGap) {
   // 2M1D2M against read "ACGT": AC - GT
   auto result = projectReadOntoReference(
      10, "ACGT", {op(2, CigarOp::MATCH), op(1, CigarOp::DELETION), op(2, CigarOp::MATCH)}
   );
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 10);
   EXPECT_EQ(result->aligned_sequence, "AC-GT");
   EXPECT_TRUE(result->insertions.empty());
}

TEST(Cigar, insertionIsRecordedNotInlined) {
   // 2M1I2M against read "ACGTT": AC [insert G at ref 7] TT
   auto result = projectReadOntoReference(
      5, "ACGTT", {op(2, CigarOp::MATCH), op(1, CigarOp::INSERTION), op(2, CigarOp::MATCH)}
   );
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 5);
   EXPECT_EQ(result->aligned_sequence, "ACTT");
   ASSERT_EQ(result->insertions.size(), 1U);
   EXPECT_EQ(result->insertions.at(0), "7:G");
}

TEST(Cigar, softClipDroppedAndDoesNotShiftOffset) {
   // 2S4M against read "TTACGT": the leading 2 bases are clipped, offset stays at pos.
   auto result =
      projectReadOntoReference(0, "TTACGT", {op(2, CigarOp::SOFT_CLIP), op(4, CigarOp::MATCH)});
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 0);
   EXPECT_EQ(result->aligned_sequence, "ACGT");
}

TEST(Cigar, referenceSkipBecomesN) {
   // 2M3N2M against read "ACGT": AC NNN GT
   auto result = projectReadOntoReference(
      0, "ACGT", {op(2, CigarOp::MATCH), op(3, CigarOp::SKIP), op(2, CigarOp::MATCH)}
   );
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->aligned_sequence, "ACNNNGT");
}

TEST(Cigar, hardClipConsumesNothing) {
   // 2H4M against read "ACGT": hard clip is absent from SEQ, so read length is 4.
   auto result =
      projectReadOntoReference(3, "ACGT", {op(2, CigarOp::HARD_CLIP), op(4, CigarOp::MATCH)});
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 3);
   EXPECT_EQ(result->aligned_sequence, "ACGT");
}

TEST(Cigar, seqMatchAndMismatchBehaveLikeMatch) {
   auto result = projectReadOntoReference(
      0, "ACGT", {op(2, CigarOp::SEQ_MATCH), op(2, CigarOp::SEQ_MISMATCH)}
   );
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->aligned_sequence, "ACGT");
}

TEST(Cigar, mismatchedReadLengthIsAnError) {
   auto result = projectReadOntoReference(0, "ACG", {op(4, CigarOp::MATCH)});
   EXPECT_FALSE(result.has_value());
}

TEST(Cigar, combinedInsertionAndDeletion) {
   // 3M1I3M2D2M against read "ACGTACGAC" (3+1+3+2 = 9 query bases).
   auto result = projectReadOntoReference(
      100,
      "ACGTACGAC",
      {op(3, CigarOp::MATCH),
       op(1, CigarOp::INSERTION),
       op(3, CigarOp::MATCH),
       op(2, CigarOp::DELETION),
       op(2, CigarOp::MATCH)}
   );
   ASSERT_TRUE(result.has_value());
   EXPECT_EQ(result->offset, 100);
   // ACG [insert T at ref 103] ACG -- AC
   EXPECT_EQ(result->aligned_sequence, "ACGACG--AC");
   ASSERT_EQ(result->insertions.size(), 1U);
   EXPECT_EQ(result->insertions.at(0), "103:T");
}
