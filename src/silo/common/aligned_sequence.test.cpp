#include "silo/common/aligned_sequence.h"

#include <string>

#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

using silo::AminoAcid;
using silo::CoverageAndMutations;
using silo::extractCoverageAndMutationsFromSequence;
using silo::Nucleotide;
using Symbol = Nucleotide::Symbol;

namespace {

CoverageAndMutations<Nucleotide> extractNuc(
   std::string_view sequence,
   size_t offset,
   std::string_view reference
) {
   auto result = extractCoverageAndMutationsFromSequence<Nucleotide>(sequence, offset, reference);
   EXPECT_TRUE(result.has_value());
   return std::move(result).value();
}

}  // namespace

TEST(AlignedSequence, identicalSequenceHasNoMutationsAndFullCoverage) {
   const auto result = extractNuc("ACGT", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 4);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, singleMutationIsRecordedWithGlobalPosition) {
   // position 1 differs: reference C, sequence G
   const auto result = extractNuc("AGGT", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 4);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 1);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::G);
}

TEST(AlignedSequence, multipleMutationsAreRecordedInOrder) {
   // positions 0 (A->C) and 2 (G->T) differ
   const auto result = extractNuc("CCTT", 0, "ACGT");

   ASSERT_EQ(result.mutations.mutations.size(), 2);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 0);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::C);
   EXPECT_EQ(result.mutations.mutations.at(1).first, 2);
   EXPECT_EQ(result.mutations.mutations.at(1).second, Symbol::T);
}

TEST(AlignedSequence, offsetShiftsPositionsAndIndexesIntoReference) {
   // sequence covers reference[2..4); position 2 mutated (G->A), position 3 matches (T)
   const auto result = extractNuc("AT", 2, "ACGT");

   EXPECT_EQ(result.coverage.start, 2);
   EXPECT_EQ(result.coverage.end, 4);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 2);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::A);
}

TEST(AlignedSequence, missingSymbolInsideCoveredRegionIsKept) {
   // N at position 1 is within the covered range and must be retained
   const auto result = extractNuc("ANGT", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 4);
   ASSERT_EQ(result.coverage.missing_positions.size(), 1);
   EXPECT_EQ(result.coverage.missing_positions.at(0), 1);
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, leadingAndTrailingMissingSymbolsAreTrimmed) {
   // reference length 6; sequence has leading and trailing N which fall outside coverage
   const auto result = extractNuc("NCGTAN", 0, "ACGTAC");

   EXPECT_EQ(result.coverage.start, 1);
   EXPECT_EQ(result.coverage.end, 5);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, onlyMissingOutsideCoverageIsTrimmedInnerKept) {
   // N at 0 and 5 trimmed, N at 2 (inside [1,5)) kept
   const auto result = extractNuc("NCNTAN", 0, "ACGTAC");

   EXPECT_EQ(result.coverage.start, 1);
   EXPECT_EQ(result.coverage.end, 5);
   ASSERT_EQ(result.coverage.missing_positions.size(), 1);
   EXPECT_EQ(result.coverage.missing_positions.at(0), 2);
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, fullyMissingSequenceHasEmptyCoverage) {
   const auto result = extractNuc("NNNN", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 0);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, emptySequenceHasEmptyCoverage) {
   const auto result = extractNuc("", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 0);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, deletionIsRecordedAsMutationToGapAndCountsAsCoverage) {
   // '-' (GAP) is not a missing symbol; it differs from reference C, so it is a mutation
   const auto result = extractNuc("A-GT", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 4);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 1);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::GAP);
}

TEST(AlignedSequence, leadingGapStartsCoverage) {
   // GAP is non-missing, so coverage starts at position 0 even with a leading gap
   const auto result = extractNuc("-CGT", 0, "ACGT");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 4);
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 0);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::GAP);
}

TEST(AlignedSequence, illegalCharacterHasError) {
   auto result = extractCoverageAndMutationsFromSequence<Nucleotide>("ACXT", 0, "ACGT");
   ASSERT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), "illegal character 'X' at position 2 in the input sequence");
}

TEST(AlignedSequence, offsetLeavesPositionsBelowOffsetWithoutCoverage) {
   // sequence covers reference[4..8); positions 0..3 are below the offset and uncovered
   const auto result = extractNuc("ACGT", 4, "AAAAACGT");

   EXPECT_EQ(result.coverage.start, 4);
   EXPECT_EQ(result.coverage.end, 8);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, offsetWithMutationUsesGlobalReferencePosition) {
   // reference[4]=A (match), reference[5]=C (sequence G -> mutation at global position 5)
   const auto result = extractNuc("AG", 4, "AAAAAC");

   EXPECT_EQ(result.coverage.start, 4);
   EXPECT_EQ(result.coverage.end, 6);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 5);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::G);
}

TEST(AlignedSequence, offsetKeepsInternalMissingAtGlobalPosition) {
   // N at global position 5 is inside the covered range and retained
   const auto result = extractNuc("ANT", 4, "AAAAACT");

   EXPECT_EQ(result.coverage.start, 4);
   EXPECT_EQ(result.coverage.end, 7);
   ASSERT_EQ(result.coverage.missing_positions.size(), 1);
   EXPECT_EQ(result.coverage.missing_positions.at(0), 5);
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, offsetTrimsLeadingAndTrailingMissing) {
   // leading N at global 4 and trailing N at global 7 fall outside coverage and are trimmed;
   // coverage starts at 5, so everything below 5 (including the offset itself) is uncovered
   const auto result = extractNuc("NCGN", 4, "AAAAACGA");

   EXPECT_EQ(result.coverage.start, 5);
   EXPECT_EQ(result.coverage.end, 7);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, offsetWithFullyMissingHasEmptyCoverage) {
   const auto result = extractNuc("NN", 4, "AAAAAA");

   EXPECT_EQ(result.coverage.start, 0);
   EXPECT_EQ(result.coverage.end, 0);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   EXPECT_TRUE(result.mutations.mutations.empty());
}

TEST(AlignedSequence, offsetWithLeadingGapCoversFromOffset) {
   // GAP is non-missing, so coverage starts at the offset even with a leading gap;
   // reference[4]=A differs from '-', so the gap is recorded as a mutation
   const auto result = extractNuc("-GT", 4, "AAAAAGT");

   EXPECT_EQ(result.coverage.start, 4);
   EXPECT_EQ(result.coverage.end, 7);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   ASSERT_EQ(result.mutations.mutations.size(), 1);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 4);
   EXPECT_EQ(result.mutations.mutations.at(0).second, Symbol::GAP);
}

TEST(AlignedSequence, largeOffsetShiftsCoverageRange) {
   const std::string reference(58, 'A');
   const auto result = extractNuc("ACGT", 50, reference);

   EXPECT_EQ(result.coverage.start, 50);
   EXPECT_EQ(result.coverage.end, 54);
   EXPECT_TRUE(result.coverage.missing_positions.empty());
   // reference is all 'A', so C, G, T are mutations at global positions 51, 52, 53
   ASSERT_EQ(result.mutations.mutations.size(), 3);
   EXPECT_EQ(result.mutations.mutations.at(0).first, 51);
   EXPECT_EQ(result.mutations.mutations.at(1).first, 52);
   EXPECT_EQ(result.mutations.mutations.at(2).first, 53);
}

TEST(AlignedSequence, worksForAminoAcidSymbolType) {
   // AminoAcid uses X as the missing symbol; verify the template generalizes.
   auto result = extractCoverageAndMutationsFromSequence<AminoAcid>("MXKL", 0, "MNKL");
   ASSERT_TRUE(result.has_value());
   const auto& value = result.value();

   EXPECT_EQ(value.coverage.start, 0);
   EXPECT_EQ(value.coverage.end, 4);
   ASSERT_EQ(value.coverage.missing_positions.size(), 1);
   EXPECT_EQ(value.coverage.missing_positions.at(0), 1);
   EXPECT_TRUE(value.mutations.mutations.empty());
}
