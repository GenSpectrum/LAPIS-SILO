#ifndef SILO_AA_SYMBOLS_H
#define SILO_AA_SYMBOLS_H

#include <array>
#include <iostream>
#include <optional>
#include <vector>

namespace silo {

// https://www.bioinformatics.org/sms/iupac.html
enum class AA_SYMBOL {
   A,  // Alanine
   B,  // Aspartic acid or Asparagine
   C,  // Cysteine
   D,  // Aspartic Acid
   E,  // Glutamic Acid
   F,  // Phenylalanine
   G,  // Glycine
   H,  // Histidine
   I,  // Isoleucine
   K,  // Lysine
   L,  // Leucine
   M,  // Methionine
   N,  // Asparagine
   P,  // Proline
   Q,  // Glutamine
   R,  // Arginine
   S,  // Serine
   T,  // Threonine
   V,  // Valine
   W,  // Tryptophan
   Y,  // Tyrosine
   Z,  // Glutamine or Glutamic acid
   X,  // Any amino acid
};

static constexpr unsigned AA_SYMBOL_COUNT = static_cast<unsigned>(AA_SYMBOL::X) + 1;

static constexpr std::array<char, AA_SYMBOL_COUNT> AA_SYMBOL_REPRESENTATION{
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L', 'M',
   'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'Y', 'Z', 'X',
};

static constexpr std::array<AA_SYMBOL, AA_SYMBOL_COUNT> AA_SYMBOLS{
   AA_SYMBOL::A, AA_SYMBOL::B, AA_SYMBOL::C, AA_SYMBOL::D, AA_SYMBOL::E, AA_SYMBOL::F,
   AA_SYMBOL::G, AA_SYMBOL::H, AA_SYMBOL::I, AA_SYMBOL::K, AA_SYMBOL::L, AA_SYMBOL::M,
   AA_SYMBOL::N, AA_SYMBOL::P, AA_SYMBOL::Q, AA_SYMBOL::R, AA_SYMBOL::S, AA_SYMBOL::T,
   AA_SYMBOL::V, AA_SYMBOL::W, AA_SYMBOL::Y, AA_SYMBOL::Z, AA_SYMBOL::X,
};

inline std::string genomeSymbolRepresentation(AA_SYMBOL symbol) {
   return std::string(1, AA_SYMBOL_REPRESENTATION.at(static_cast<unsigned>(symbol)));
}

inline std::optional<AA_SYMBOL> toAASymbol(char character) {
   switch (character) {
      case 'A':
         return AA_SYMBOL::A;
      case 'B':
         return AA_SYMBOL::B;
      case 'C':
         return AA_SYMBOL::C;
      case 'D':
         return AA_SYMBOL::D;
      case 'E':
         return AA_SYMBOL::E;
      case 'F':
         return AA_SYMBOL::F;
      case 'G':
         return AA_SYMBOL::G;
      case 'H':
         return AA_SYMBOL::H;
      case 'I':
         return AA_SYMBOL::I;
      case 'K':
         return AA_SYMBOL::K;
      case 'L':
         return AA_SYMBOL::L;
      case 'M':
         return AA_SYMBOL::M;
      case 'N':
         return AA_SYMBOL::N;
      case 'P':
         return AA_SYMBOL::P;
      case 'Q':
         return AA_SYMBOL::Q;
      case 'R':
         return AA_SYMBOL::R;
      case 'S':
         return AA_SYMBOL::S;
      case 'T':
         return AA_SYMBOL::T;
      case 'V':
         return AA_SYMBOL::V;
      case 'W':
         return AA_SYMBOL::W;
      case 'Y':
         return AA_SYMBOL::Y;
      case 'Z':
         return AA_SYMBOL::Z;
      case 'X':
         return AA_SYMBOL::X;
      default:
         return std::nullopt;
   }
}
}  // namespace silo

#endif  // SILO_AA_SYMBOLS_H
