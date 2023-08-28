#include "silo/common/aa_symbols.h"

#include <gtest/gtest.h>

TEST(AminoAcidSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(silo::AminoAcid::COUNT, silo::AminoAcid::SYMBOLS.size());
}

TEST(AminoAcidSymbol, conversionFromCharacter) {
   EXPECT_EQ(silo::AminoAcid::charToSymbol('-'), silo::AminoAcid::Symbol::GAP);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('A'), silo::AminoAcid::Symbol::A);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('N'), silo::AminoAcid::Symbol::N);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('J'), std::nullopt);
   EXPECT_EQ(silo::AminoAcid::charToSymbol(')'), std::nullopt);
}

TEST(AminoAcidSymbol, findIllegalAminoAcidChar) {
   EXPECT_EQ(silo::AminoAcid::findIllegalChar("ACGT"), std::nullopt);
   EXPECT_EQ(silo::AminoAcid::findIllegalChar("ACGTJ"), 'J');
}