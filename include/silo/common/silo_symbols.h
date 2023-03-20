#ifndef SILO_H
#define SILO_H

#include <array>
#include <iostream>

namespace silo {

static constexpr unsigned GENOME_LENGTH = 29903;

// https://www.bioinformatics.org/sms/iupac.html
enum GENOME_SYMBOL {
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

static constexpr unsigned SYMBOL_COUNT = static_cast<unsigned>(GENOME_SYMBOL::N) + 1;

static constexpr std::array<char, SYMBOL_COUNT> SYMBOL_REPRESENTATION{
   '-', 'A', 'C', 'G', 'T', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V', 'N'};

static_assert(SYMBOL_REPRESENTATION[static_cast<unsigned>(GENOME_SYMBOL::N)] == 'N');

inline GENOME_SYMBOL toNucleotideSymbol(char character) {
   GENOME_SYMBOL symbol = GENOME_SYMBOL::GAP;
   switch (character) {
      case '.':
      case '-':
         symbol = GENOME_SYMBOL::GAP;
         break;
      case 'A':
         symbol = GENOME_SYMBOL::A;
         break;
      case 'C':
         symbol = GENOME_SYMBOL::C;
         break;
      case 'G':
         symbol = GENOME_SYMBOL::G;
         break;
      case 'T':
      case 'U':
         symbol = GENOME_SYMBOL::T;
         break;
      case 'R':
         symbol = GENOME_SYMBOL::R;
         break;
      case 'Y':
         symbol = GENOME_SYMBOL::Y;
         break;
      case 'S':
         symbol = GENOME_SYMBOL::S;
         break;
      case 'W':
         symbol = GENOME_SYMBOL::W;
         break;
      case 'K':
         symbol = GENOME_SYMBOL::K;
         break;
      case 'M':
         symbol = GENOME_SYMBOL::M;
         break;
      case 'B':
         symbol = GENOME_SYMBOL::B;
         break;
      case 'D':
         symbol = GENOME_SYMBOL::D;
         break;
      case 'H':
         symbol = GENOME_SYMBOL::H;
         break;
      case 'V':
         symbol = GENOME_SYMBOL::V;
         break;
      case 'N':
         symbol = GENOME_SYMBOL::N;
         break;
      default:
         std::cerr << "unrecognized symbol " << character << std::endl;
   }
   return symbol;
}
}  // namespace silo

#endif  // SILO_H
