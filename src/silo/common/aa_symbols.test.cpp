#include "silo/common/aa_symbols.h"

#include <gtest/gtest.h>

TEST(AA_SYMBOL, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::count, silo::Util<silo::AA_SYMBOL>::symbols.size());
}

TEST(AA_SYMBOL, conversionFromCharacter) {
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::charToSymbol('-'), silo::AA_SYMBOL::GAP);
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::charToSymbol('A'), silo::AA_SYMBOL::A);
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::charToSymbol('N'), silo::AA_SYMBOL::N);
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::charToSymbol('J'), std::nullopt);
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::charToSymbol(')'), std::nullopt);
}

TEST(AA_SYMBOL, findIllegalAminoAcidChar) {
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::findIllegalChar("ACGT"), std::nullopt);
   EXPECT_EQ(silo::Util<silo::AA_SYMBOL>::findIllegalChar("ACGTJ"), 'J');
}