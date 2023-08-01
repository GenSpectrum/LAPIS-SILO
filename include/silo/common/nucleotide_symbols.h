#ifndef SILO_NUCLEOTIDE_SYMBOLS_H
#define SILO_NUCLEOTIDE_SYMBOLS_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace silo {

/// https://www.bioinformatics.org/sms2/iupac.html
enum class NUCLEOTIDE_SYMBOL : char {
   GAP,  // -, GAP
   A,    // Adenine
   C,    // Cytosine
   G,    // Guanine
   T,    // (or U) Thymine (or Uracil)
   R,    // A or G
   Y,    // C or T
   S,    // G or C
   W,    // A or T
   K,    // G or T
   M,    // A or C
   B,    // C or G or T
   D,    // A or G or T
   H,    // A or C or T
   V,    // A or C or G
   N,    // any base
};

static constexpr uint32_t NUC_SYMBOL_COUNT = static_cast<uint32_t>(NUCLEOTIDE_SYMBOL::N) + 1;

static constexpr std::array<NUCLEOTIDE_SYMBOL, NUC_SYMBOL_COUNT> NUC_SYMBOLS{
   NUCLEOTIDE_SYMBOL::GAP,
   NUCLEOTIDE_SYMBOL::A,
   NUCLEOTIDE_SYMBOL::C,
   NUCLEOTIDE_SYMBOL::G,
   NUCLEOTIDE_SYMBOL::T,
   NUCLEOTIDE_SYMBOL::R,
   NUCLEOTIDE_SYMBOL::Y,
   NUCLEOTIDE_SYMBOL::S,
   NUCLEOTIDE_SYMBOL::W,
   NUCLEOTIDE_SYMBOL::K,
   NUCLEOTIDE_SYMBOL::M,
   NUCLEOTIDE_SYMBOL::B,
   NUCLEOTIDE_SYMBOL::D,
   NUCLEOTIDE_SYMBOL::H,
   NUCLEOTIDE_SYMBOL::V,
   NUCLEOTIDE_SYMBOL::N,
};

char nucleotideSymbolToChar(NUCLEOTIDE_SYMBOL symbol);

std::optional<NUCLEOTIDE_SYMBOL> charToNucleotideSymbol(char character);

std::optional<std::vector<NUCLEOTIDE_SYMBOL>> stringToNucleotideSymbolVector(
   const std::string& nucleotides
);

std::optional<char> findIllegalNucleotideChar(const std::string& nucleotides);

}  // namespace silo

#endif  // SILO_NUCLEOTIDE_SYMBOLS_H
