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

inline char nucleotideSymbolToChar(NUCLEOTIDE_SYMBOL symbol) {
   switch (symbol) {
      case NUCLEOTIDE_SYMBOL::GAP:
         return '-';
      case NUCLEOTIDE_SYMBOL::A:
         return 'A';
      case NUCLEOTIDE_SYMBOL::C:
         return 'C';
      case NUCLEOTIDE_SYMBOL::G:
         return 'G';
      case NUCLEOTIDE_SYMBOL::T:
         return 'T';
      case NUCLEOTIDE_SYMBOL::R:
         return 'R';
      case NUCLEOTIDE_SYMBOL::Y:
         return 'Y';
      case NUCLEOTIDE_SYMBOL::S:
         return 'S';
      case NUCLEOTIDE_SYMBOL::W:
         return 'W';
      case NUCLEOTIDE_SYMBOL::K:
         return 'K';
      case NUCLEOTIDE_SYMBOL::M:
         return 'M';
      case NUCLEOTIDE_SYMBOL::B:
         return 'B';
      case NUCLEOTIDE_SYMBOL::D:
         return 'D';
      case NUCLEOTIDE_SYMBOL::H:
         return 'H';
      case NUCLEOTIDE_SYMBOL::V:
         return 'V';
      case NUCLEOTIDE_SYMBOL::N:
         return 'N';
   }
}

inline std::optional<NUCLEOTIDE_SYMBOL> charToNucleotideSymbol(char character) {
   switch (character) {
      case '.':
      case '-':
         return NUCLEOTIDE_SYMBOL::GAP;
      case 'A':
         return NUCLEOTIDE_SYMBOL::A;
      case 'C':
         return NUCLEOTIDE_SYMBOL::C;
      case 'G':
         return NUCLEOTIDE_SYMBOL::G;
      case 'T':
      case 'U':
         return NUCLEOTIDE_SYMBOL::T;
      case 'R':
         return NUCLEOTIDE_SYMBOL::R;
      case 'Y':
         return NUCLEOTIDE_SYMBOL::Y;
      case 'S':
         return NUCLEOTIDE_SYMBOL::S;
      case 'W':
         return NUCLEOTIDE_SYMBOL::W;
      case 'K':
         return NUCLEOTIDE_SYMBOL::K;
      case 'M':
         return NUCLEOTIDE_SYMBOL::M;
      case 'B':
         return NUCLEOTIDE_SYMBOL::B;
      case 'D':
         return NUCLEOTIDE_SYMBOL::D;
      case 'H':
         return NUCLEOTIDE_SYMBOL::H;
      case 'V':
         return NUCLEOTIDE_SYMBOL::V;
      case 'N':
         return NUCLEOTIDE_SYMBOL::N;
      default:
         return std::nullopt;
   }
}

inline std::optional<std::vector<NUCLEOTIDE_SYMBOL>> stringToNucleotideSymbolVector(
   const std::string& nucleotides
) {
   const size_t size = nucleotides.size();
   std::vector<NUCLEOTIDE_SYMBOL> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = charToNucleotideSymbol(nucleotides[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

}  // namespace silo

#endif  // SILO_NUCLEOTIDE_SYMBOLS_H
