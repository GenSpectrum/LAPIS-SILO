#ifndef SILO_H
#define SILO_H

#include <array>
#include <iostream>

#include <spdlog/spdlog.h>

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
   switch (character) {
      case '.':
      case '-':
         return GENOME_SYMBOL::GAP;
      case 'A':
         return GENOME_SYMBOL::A;
      case 'C':
         return GENOME_SYMBOL::C;
      case 'G':
         return GENOME_SYMBOL::G;
      case 'T':
      case 'U':
         return GENOME_SYMBOL::T;
      case 'R':
         return GENOME_SYMBOL::R;
      case 'Y':
         return GENOME_SYMBOL::Y;
      case 'S':
         return GENOME_SYMBOL::S;
      case 'W':
         return GENOME_SYMBOL::W;
      case 'K':
         return GENOME_SYMBOL::K;
      case 'M':
         return GENOME_SYMBOL::M;
      case 'B':
         return GENOME_SYMBOL::B;
      case 'D':
         return GENOME_SYMBOL::D;
      case 'H':
         return GENOME_SYMBOL::H;
      case 'V':
         return GENOME_SYMBOL::V;
      case 'N':
         return GENOME_SYMBOL::N;
      default:
         SPDLOG_ERROR("unrecognized symbol {}", character);
         return GENOME_SYMBOL::GAP;
   }
}
}  // namespace silo

#endif  // SILO_H
