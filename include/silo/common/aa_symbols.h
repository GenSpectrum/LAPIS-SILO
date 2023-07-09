#ifndef SILO_AA_SYMBOLS_H
#define SILO_AA_SYMBOLS_H

#include <array>
#include <iostream>
#include <optional>

namespace silo {

/// https://www.bioinformatics.org/sms2/iupac.html
enum class AA_SYMBOL : char {
   GAP,   // - Gap in sequence
   A,     // Alanine
   C,     // Cysteine
   D,     // Aspartic Acid
   E,     // Glutamic Acid
   F,     // Phenylalanine
   G,     // Glycine
   H,     // Histidine
   I,     // Isoleucine
   K,     // Lysine
   L,     // Leucine
   M,     // Methionine
   N,     // Asparagine
   P,     // Proline
   Q,     // Glutamine
   R,     // Arginine
   S,     // Serine
   T,     // Threonine
   V,     // Valine
   W,     // Tryptophan
   Y,     // Tyrosine
   B,     // Aspartic acid or Asparagine
   Z,     // Glutamine or Glutamic acid
   STOP,  // Stop codon
   X,     // Any amino acid
};

static constexpr uint32_t AA_SYMBOL_COUNT = 25;
static constexpr uint32_t CODING_AA_SYMBOL_COUNT = 21;

static constexpr std::array<AA_SYMBOL, AA_SYMBOL_COUNT> AA_SYMBOLS{
   AA_SYMBOL::GAP, AA_SYMBOL::A, AA_SYMBOL::C, AA_SYMBOL::D, AA_SYMBOL::E,
   AA_SYMBOL::F,   AA_SYMBOL::G, AA_SYMBOL::H, AA_SYMBOL::I, AA_SYMBOL::K,
   AA_SYMBOL::L,   AA_SYMBOL::M, AA_SYMBOL::N, AA_SYMBOL::P, AA_SYMBOL::Q,
   AA_SYMBOL::R,   AA_SYMBOL::S, AA_SYMBOL::T, AA_SYMBOL::V, AA_SYMBOL::W,
   AA_SYMBOL::Y,   AA_SYMBOL::B, AA_SYMBOL::Z, AA_SYMBOL::X, AA_SYMBOL::STOP,
};

static constexpr std::array<AA_SYMBOL, CODING_AA_SYMBOL_COUNT> VALID_AA_SYMBOL_READS{
   AA_SYMBOL::GAP, AA_SYMBOL::A, AA_SYMBOL::C, AA_SYMBOL::D, AA_SYMBOL::E, AA_SYMBOL::F,
   AA_SYMBOL::G,   AA_SYMBOL::H, AA_SYMBOL::I, AA_SYMBOL::K, AA_SYMBOL::L, AA_SYMBOL::M,
   AA_SYMBOL::N,   AA_SYMBOL::P, AA_SYMBOL::Q, AA_SYMBOL::R, AA_SYMBOL::S, AA_SYMBOL::T,
   AA_SYMBOL::V,   AA_SYMBOL::W, AA_SYMBOL::Y,
};

inline char aaSymbolToChar(AA_SYMBOL symbol) {
   switch (symbol) {
      case AA_SYMBOL::GAP:
         return '-';
      case AA_SYMBOL::A:
         return 'A';
      case AA_SYMBOL::C:
         return 'C';
      case AA_SYMBOL::D:
         return 'D';
      case AA_SYMBOL::E:
         return 'E';
      case AA_SYMBOL::F:
         return 'F';
      case AA_SYMBOL::G:
         return 'G';
      case AA_SYMBOL::H:
         return 'H';
      case AA_SYMBOL::I:
         return 'I';
      case AA_SYMBOL::K:
         return 'K';
      case AA_SYMBOL::L:
         return 'L';
      case AA_SYMBOL::N:
         return 'N';
      case AA_SYMBOL::M:
         return 'M';
      case AA_SYMBOL::P:
         return 'P';
      case AA_SYMBOL::Q:
         return 'Q';
      case AA_SYMBOL::R:
         return 'R';
      case AA_SYMBOL::S:
         return 'S';
      case AA_SYMBOL::T:
         return 'T';
      case AA_SYMBOL::V:
         return 'V';
      case AA_SYMBOL::W:
         return 'W';
      case AA_SYMBOL::Y:
         return 'Y';
      case AA_SYMBOL::B:
         return 'B';
      case AA_SYMBOL::Z:
         return 'Z';
      case AA_SYMBOL::X:
         return 'X';
      case AA_SYMBOL::STOP:
         return '*';
   }
}

inline std::optional<AA_SYMBOL> charToAASymbol(char character) {
   switch (character) {
      case '-':
         return AA_SYMBOL::GAP;
      case 'A':
         return AA_SYMBOL::A;
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
      case 'N':
         return AA_SYMBOL::N;
      case 'M':
         return AA_SYMBOL::M;
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
      case 'B':
         return AA_SYMBOL::B;
      case 'Z':
         return AA_SYMBOL::Z;
      case 'X':
         return AA_SYMBOL::X;
      case '*':
         return AA_SYMBOL::STOP;
      default:
         return std::nullopt;
   }
}
}  // namespace silo

#endif  // SILO_AA_SYMBOLS_H
