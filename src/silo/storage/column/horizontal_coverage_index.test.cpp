#include "silo/storage/column/horizontal_coverage_index.h"

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"

namespace silo::storage::column {

// Test fixture for HorizontalCoverageIndex with Nucleotide
class HorizontalCoverageIndexTest : public ::testing::Test {
  protected:
   static constexpr size_t GENOME_LENGTH = 100;

   void SetUp() override { index = std::make_unique<HorizontalCoverageIndex>(); }

   std::unique_ptr<HorizontalCoverageIndex> index;
};

TEST_F(HorizontalCoverageIndexTest, InsertMultipleNullSequences) {
   EXPECT_NO_THROW({
      index->insertNullSequence();
      index->insertNullSequence();
      index->insertNullSequence();
   });
}

TEST_F(HorizontalCoverageIndexTest, NullSequenceIsUncoveredAtAllPositions) {
   index->insertNullSequence();  // Sequence 0 is null

   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<51>(0).at(50), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<99>(1).at(98), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, MultipleNullSequencesAllUncovered) {
   index->insertNullSequence();  // Sequence 0
   index->insertNullSequence();  // Sequence 1
   index->insertNullSequence();  // Sequence 2

   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<2>(50).at(0), (roaring::Roaring{}));
}

// Coverage Insertion Tests
TEST_F(HorizontalCoverageIndexTest, InsertSingleSequenceAtOffsetZero) {
   std::string sequence = "ACGT";
   EXPECT_NO_THROW(index->insertSequenceCoverage<Nucleotide>(sequence, 0));
}

TEST_F(HorizontalCoverageIndexTest, InsertSequenceWithOffset) {
   std::string sequence = "ACGT";
   EXPECT_NO_THROW(index->insertSequenceCoverage<Nucleotide>(sequence, 10));
}

TEST_F(HorizontalCoverageIndexTest, InsertEmptySequence) {
   std::string sequence = "";
   EXPECT_NO_THROW(index->insertSequenceCoverage<Nucleotide>(sequence, 0));
}

TEST_F(HorizontalCoverageIndexTest, InsertMultipleSequences) {
   EXPECT_NO_THROW({
      index->insertSequenceCoverage<Nucleotide>("ACGT", 0);
      index->insertSequenceCoverage<Nucleotide>("TGCA", 5);
      index->insertSequenceCoverage<Nucleotide>("AAAA", 10);
   });
}

// Bitmap Retrieval Tests - Covered Positions
TEST_F(HorizontalCoverageIndexTest, SequenceNotInBitmapForCoveredPositions) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);  // Covers positions 0-3

   // Sequence 0 is covered at positions 0-3, so it should appear in bitmap
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(1).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(3).at(0), roaring::Roaring{0});
}

TEST_F(HorizontalCoverageIndexTest, SequenceInBitmapForUncoveredPositions) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);  // Covers positions 0-3

   // Sequence 0 is NOT covered at positions 4+, so it should NOT appear in bitmap
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(4).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(10).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(50).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, MultipleSequencesWithOverlap) {
   index->insertSequenceCoverage<Nucleotide>("ACGTACGT", 0);  // Sequence 0 covers 0-7
   index->insertSequenceCoverage<Nucleotide>("TGCA", 5);      // Sequence 1 covers 5-8
   index->insertSequenceCoverage<Nucleotide>("AAAA", 10);     // Sequence 2 covers 10-13

   // Position 6: covered by sequences 0 and 1, so only 2 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), (roaring::Roaring{0, 1}));

   // Position 5: covered by sequences 0 and 1, so only 2 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{0, 1}));

   // Position 7: covered by sequences 0 and 1, so only 2 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(7).at(0), (roaring::Roaring{0, 1}));

   // Position 8: covered by sequence 1 only, so 0 and 2 are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(8).at(0), (roaring::Roaring{1}));

   // Position 11: covered by sequence 2 only, so 0 and 1 are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(11).at(0), (roaring::Roaring{2}));
}

TEST_F(HorizontalCoverageIndexTest, NonOverlappingSequences) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);   // Covers 0-3
   index->insertSequenceCoverage<Nucleotide>("TGCA", 10);  // Covers 10-13

   // Position 2: covered by 0, uncovered by 1
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), roaring::Roaring{0});

   // Position 11: covered by 1, uncovered by 0
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(11).at(0), roaring::Roaring{1});

   // Position 5: uncovered by both
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{}));
}

// N Character Tests - N counts as uncovered!
TEST_F(HorizontalCoverageIndexTest, SequenceWithNIsUncoveredAtNPosition) {
   index->insertSequenceCoverage<Nucleotide>("ACNGT", 0);  // Position 2 has N

   // Positions 0, 1, 3, 4 are covered (A, C, G, T)
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(1).at(0), (roaring::Roaring{0}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(3).at(0), (roaring::Roaring{0}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(4).at(0), (roaring::Roaring{0}));

   // Position 2 has N, so sequence 0 is uncovered there
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), (roaring::Roaring{}));
}

TEST_F(HorizontalCoverageIndexTest, SequenceWithMultipleNs) {
   index->insertSequenceCoverage<Nucleotide>("ANNNGTA", 0);  // Positions 1, 2, 3 have N

   // Positions with actual nucleotides are covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{0});  // A
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(4).at(0), roaring::Roaring{0});  // G
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), roaring::Roaring{0});  // T
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), roaring::Roaring{0});  // A

   // Positions with N are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(1).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(3).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, SequenceWithAllNs) {
   index->insertSequenceCoverage<Nucleotide>("NNNN", 5);

   // All positions with N are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(7).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(8).at(0), roaring::Roaring{});

   // Positions outside the sequence range are also uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, MultipleSequencesWithNAtSamePosition) {
   index->insertSequenceCoverage<Nucleotide>("ACNGT", 0);  // N at position 2
   index->insertSequenceCoverage<Nucleotide>("AANAT", 0);  // N at position 2
   index->insertSequenceCoverage<Nucleotide>("ACGGT", 0);  // No N (G at position 2)

   // At position 2: sequences 0 and 1 have N (uncovered), sequence 2 has G (covered)
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), (roaring::Roaring{2}));

   // At position 0: all have actual nucleotides (covered)
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0, 1, 2}));
}

TEST_F(HorizontalCoverageIndexTest, SequenceWithNAndOffset) {
   index->insertSequenceCoverage<Nucleotide>("ACNGT", 10);  // N at position 12

   // Position 12 has N, so sequence 0 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(12).at(0), roaring::Roaring{});

   // Other positions in the sequence are covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(10).at(0), roaring::Roaring{0});  // A
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(11).at(0), roaring::Roaring{0});  // C
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(13).at(0), roaring::Roaring{0});  // G
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(14).at(0), roaring::Roaring{0});  // T
}

TEST_F(HorizontalCoverageIndexTest, SequenceWithMixedCase) {
   index->insertSequenceCoverage<Nucleotide>("AcGt", 0);
   EXPECT_NO_THROW(index->insertSequenceCoverage<Nucleotide>("AcGt", 0));
}

TEST_F(HorizontalCoverageIndexTest, SequenceWithLowercaseN) {
   index->insertSequenceCoverage<Nucleotide>("ACnGT", 0);  // lowercase n at position 2

   // Lowercase n should also count as uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), roaring::Roaring{});
}

// Edge Cases
TEST_F(HorizontalCoverageIndexTest, EmptyIndexAllPositionsEmpty) {
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(50).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, GetBitmapAtGenomeBoundary) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);

   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 1).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, GetBitmapBeyondGenomeLength) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);

   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH).at(0), roaring::Roaring{});
   EXPECT_EQ(
      index->getCoverageBitmapForPositions<1>(GENOME_LENGTH + 100).at(0), roaring::Roaring{}
   );
}

TEST_F(HorizontalCoverageIndexTest, InsertSequenceAtEndOfGenome) {
   index->insertSequenceCoverage<Nucleotide>("ACG", GENOME_LENGTH - 3);

   // Sequence covers last 3 positions
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 3).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 2).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 1).at(0), roaring::Roaring{0});

   // But not positions before
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 4).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, InsertSequenceWithNAtEndOfGenome) {
   index->insertSequenceCoverage<Nucleotide>("ACN", GENOME_LENGTH - 3);

   // Positions with A and C are covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 3).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 2).at(0), roaring::Roaring{0});

   // Position with N is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH - 1).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, InsertSequenceExtendingBeyondGenome) {
   // Sequence extends beyond genome length
   EXPECT_NO_THROW(index->insertSequenceCoverage<Nucleotide>("ACGTACGT", GENOME_LENGTH - 2));
}

// Mixed Null and Non-Null Sequences
TEST_F(HorizontalCoverageIndexTest, MixedNullAndCoveredSequences) {
   index->insertSequenceCoverage<Nucleotide>("ACGT", 0);  // Sequence 0: covers 0-3
   index->insertNullSequence();                           // Sequence 1: null (uncovered everywhere)
   index->insertSequenceCoverage<Nucleotide>("TGCA", 0);  // Sequence 2: covers 0-3

   // At positions 0-3: sequences 0 and 2 are covered, sequence 1 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(1).at(0), (roaring::Roaring{0, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), (roaring::Roaring{0, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(3).at(0), (roaring::Roaring{0, 2}));

   // At position 10: all are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(10).at(0), (roaring::Roaring{}));
}

TEST_F(HorizontalCoverageIndexTest, MultipleNullsWithCoverage) {
   index->insertNullSequence();                           // Sequence 0: null
   index->insertNullSequence();                           // Sequence 1: null
   index->insertSequenceCoverage<Nucleotide>("ACGT", 5);  // Sequence 2: covers 5-8
   index->insertNullSequence();                           // Sequence 3: null

   // At position 0: all nulls are uncovered, seq 2 is also uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{}));

   // At positions 5-8: nulls are uncovered, seq 2 is covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), (roaring::Roaring{2}));
}

TEST_F(HorizontalCoverageIndexTest, MixedNullCoverageAndNCharacters) {
   index->insertSequenceCoverage<Nucleotide>("ACNGT", 0);  // Sequence 0: N at position 2
   index->insertNullSequence();                            // Sequence 1: null everywhere
   index->insertSequenceCoverage<Nucleotide>("ACGGT", 0);  // Sequence 2: fully covered 0-4

   // Position 2: seq 0 has N (uncovered), seq 1 is null (uncovered), seq 2 has G (covered)
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), (roaring::Roaring{2}));

   // Position 0: seq 0 has A (covered), seq 1 is null (uncovered), seq 2 has A (covered)
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0, 2}));
}

// Overlapping Coverage Tests
TEST_F(HorizontalCoverageIndexTest, CompletelyOverlappingSequences) {
   index->insertSequenceCoverage<Nucleotide>("AAAA", 5);
   index->insertSequenceCoverage<Nucleotide>("TTTT", 5);
   index->insertSequenceCoverage<Nucleotide>("GGGG", 5);

   // At positions 5-8: all three sequences are covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{0, 1, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), (roaring::Roaring{0, 1, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(7).at(0), (roaring::Roaring{0, 1, 2}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(8).at(0), (roaring::Roaring{0, 1, 2}));

   // At position 4: all three sequences are uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(4).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, OverlappingSequencesWithN) {
   index->insertSequenceCoverage<Nucleotide>("ANNN", 5);  // Only position 5 covered
   index->insertSequenceCoverage<Nucleotide>("NANN", 5);  // Only position 6 covered
   index->insertSequenceCoverage<Nucleotide>("NNAN", 5);  // Only position 7 covered

   // Position 5: seq 0 covered, seq 1 and 2 have N
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{0}));

   // Position 6: seq 1 covered, seq 0 and 2 have N
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(6).at(0), (roaring::Roaring{1}));

   // Position 7: seq 2 covered, seq 0 and 1 have N
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(7).at(0), (roaring::Roaring{2}));

   // Position 8: all have N
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(8).at(0), (roaring::Roaring{}));
}

TEST_F(HorizontalCoverageIndexTest, PartiallyOverlappingSequences) {
   index->insertSequenceCoverage<Nucleotide>("AAAAAAAA", 0);  // 0-7
   index->insertSequenceCoverage<Nucleotide>("TTTTTTTT", 4);  // 4-11
   index->insertSequenceCoverage<Nucleotide>("GGGGGGGG", 8);  // 8-15

   // Position 2: only seq 0 is covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(2).at(0), (roaring::Roaring{0}));

   // Position 5: seq 0 and 1 are covered, seq 2 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{0, 1}));

   // Position 9: seq 1 and 2 are covered, seq 0 is uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(9).at(0), (roaring::Roaring{1, 2}));

   // Position 14: only seq 2 is covered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(14).at(0), (roaring::Roaring{2}));
}

TEST_F(HorizontalCoverageIndexTest, AdjacentNonOverlappingSequences) {
   index->insertSequenceCoverage<Nucleotide>("AAAA", 0);  // 0-3
   index->insertSequenceCoverage<Nucleotide>("TTTT", 4);  // 4-7
   index->insertSequenceCoverage<Nucleotide>("GGGG", 8);  // 8-11

   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(3).at(0), (roaring::Roaring{0}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(4).at(0), (roaring::Roaring{1}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(7).at(0), (roaring::Roaring{1}));
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(8).at(0), (roaring::Roaring{2}));
}

// Large Dataset Tests
TEST_F(HorizontalCoverageIndexTest, ManySequences) {
   constexpr size_t NUM_SEQUENCES = 1000;

   for (size_t i = 0; i < NUM_SEQUENCES; ++i) {
      index->insertSequenceCoverage<Nucleotide>("ACGT", 0);
   }
   roaring::Roaring full_bitmap;
   full_bitmap.addRange(0, NUM_SEQUENCES);

   // All sequences cover position 0, so bitmap should be full
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), full_bitmap);

   // All sequences are uncovered at position 10
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(10).at(0), roaring::Roaring{});
}

TEST_F(HorizontalCoverageIndexTest, ManySequencesAtDifferentPositions) {
   constexpr size_t NUM_SEQUENCES = 100;

   for (size_t i = 0; i < NUM_SEQUENCES; ++i) {
      index->insertSequenceCoverage<Nucleotide>("A", i);  // Each sequence covers one position
   }

   for (uint32_t pos = 0; pos < NUM_SEQUENCES; ++pos) {
      // At each position, only one sequence is covered, all others are uncovered
      EXPECT_EQ(index->getCoverageBitmapForPositions<1>(pos).at(0), roaring::Roaring{pos});
   }
}

TEST_F(HorizontalCoverageIndexTest, LongSequence) {
   std::string long_sequence(GENOME_LENGTH / 2, 'A');
   index->insertSequenceCoverage<Nucleotide>(long_sequence, 0);

   // Verify coverage
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{0});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH / 4).at(0), roaring::Roaring{0});
   EXPECT_EQ(
      index->getCoverageBitmapForPositions<1>(GENOME_LENGTH / 2 - 1).at(0), roaring::Roaring{0}
   );

   // Verify uncovered
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(GENOME_LENGTH / 2).at(0), roaring::Roaring{});
}

// Sequence Order Tests
TEST_F(HorizontalCoverageIndexTest, InsertSequencesInOrder) {
   index->insertSequenceCoverage<Nucleotide>("A", 0);
   index->insertSequenceCoverage<Nucleotide>("T", 0);
   index->insertSequenceCoverage<Nucleotide>("G", 0);

   // All covered at position 0
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0, 1, 2}));

   // All uncovered at position 5
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{}));
}

TEST_F(HorizontalCoverageIndexTest, InsertSequencesOutOfOrder) {
   index->insertSequenceCoverage<Nucleotide>("G", 0);
   index->insertSequenceCoverage<Nucleotide>("A", 0);
   index->insertSequenceCoverage<Nucleotide>("T", 0);

   // All covered at position 0
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), (roaring::Roaring{0, 1, 2}));

   // All uncovered at position 5
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), (roaring::Roaring{}));
}

// Empty Sequence Tests
TEST_F(HorizontalCoverageIndexTest, EmptySequenceIsAlwaysUncovered) {
   index->insertSequenceCoverage<Nucleotide>("", 5);

   // Empty sequence doesn't cover anything, so it's uncovered everywhere
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(0).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(5).at(0), roaring::Roaring{});
   EXPECT_EQ(index->getCoverageBitmapForPositions<1>(10).at(0), roaring::Roaring{});
}

}  // namespace silo::storage::column
