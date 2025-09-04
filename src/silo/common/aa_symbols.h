#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "silo/common/symbol_map.h"
#include "silo/schema/database_schema.h"

namespace silo {

namespace storage::column {
template <typename SymbolType>
class SequenceColumnPartition;
}

class AminoAcid {
  public:
   /// https://www.bioinformatics.org/sms2/iupac.html
   enum class Symbol : char {
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

   static constexpr schema::ColumnType COLUMN_TYPE = schema::ColumnType::AMINO_ACID_SEQUENCE;
   using Column = storage::column::SequenceColumnPartition<AminoAcid>;

   static constexpr uint32_t COUNT = 25;

   static constexpr std::string_view SYMBOL_NAME = "AminoAcid";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "amino acid";
   static constexpr std::string_view PREFIX = "aa_";

   static constexpr std::array<Symbol, COUNT> SYMBOLS{
      Symbol::GAP, Symbol::A, Symbol::C, Symbol::D,    Symbol::E, Symbol::F, Symbol::G,
      Symbol::H,   Symbol::I, Symbol::K, Symbol::L,    Symbol::M, Symbol::N, Symbol::P,
      Symbol::Q,   Symbol::R, Symbol::S, Symbol::T,    Symbol::V, Symbol::W, Symbol::Y,
      Symbol::B,   Symbol::Z, Symbol::X, Symbol::STOP,
   };

   static constexpr std::array<Symbol, 22> VALID_MUTATION_SYMBOLS{
      AminoAcid::Symbol::GAP,   // Gap, deletion in sequence
      AminoAcid::Symbol::A,     // Alanine
      AminoAcid::Symbol::C,     // Cysteine
      AminoAcid::Symbol::D,     // Aspartic Acid
      AminoAcid::Symbol::E,     // Glutamic Acid
      AminoAcid::Symbol::F,     // Phenylalanine
      AminoAcid::Symbol::G,     // Glycine
      AminoAcid::Symbol::H,     // Histidine
      AminoAcid::Symbol::I,     // Isoleucine
      AminoAcid::Symbol::K,     // Lysine
      AminoAcid::Symbol::L,     // Leucine
      AminoAcid::Symbol::M,     // Methionine
      AminoAcid::Symbol::N,     // Asparagine
      AminoAcid::Symbol::P,     // Proline
      AminoAcid::Symbol::Q,     // Glutamine
      AminoAcid::Symbol::R,     // Arginine
      AminoAcid::Symbol::S,     // Serine
      AminoAcid::Symbol::T,     // Threonine
      AminoAcid::Symbol::V,     // Valine
      AminoAcid::Symbol::W,     // Tryptophan
      AminoAcid::Symbol::Y,     // Tyrosine
      AminoAcid::Symbol::STOP,  // Stop codon, Star-character in sequence
   };

   static constexpr std::array<Symbol, 3> INVALID_MUTATION_SYMBOLS{
      AminoAcid::Symbol::B,  // Aspartic acid or Asparagine
      AminoAcid::Symbol::Z,  // Glutamine or Glutamic acid
      AminoAcid::Symbol::X,  // Any amino acid
   };

   static_assert(INVALID_MUTATION_SYMBOLS.size() + VALID_MUTATION_SYMBOLS.size() == SYMBOLS.size());

   static const silo::SymbolMap<AminoAcid, std::vector<AminoAcid::Symbol>> AMBIGUITY_SYMBOLS;

   static constexpr Symbol SYMBOL_MISSING = Symbol::X;

   static inline char symbolToChar(AminoAcid::Symbol symbol) {
      switch (symbol) {
         case AminoAcid::Symbol::GAP:
            return '-';
         case AminoAcid::Symbol::A:
            return 'A';
         case AminoAcid::Symbol::C:
            return 'C';
         case AminoAcid::Symbol::D:
            return 'D';
         case AminoAcid::Symbol::E:
            return 'E';
         case AminoAcid::Symbol::F:
            return 'F';
         case AminoAcid::Symbol::G:
            return 'G';
         case AminoAcid::Symbol::H:
            return 'H';
         case AminoAcid::Symbol::I:
            return 'I';
         case AminoAcid::Symbol::K:
            return 'K';
         case AminoAcid::Symbol::L:
            return 'L';
         case AminoAcid::Symbol::N:
            return 'N';
         case AminoAcid::Symbol::M:
            return 'M';
         case AminoAcid::Symbol::P:
            return 'P';
         case AminoAcid::Symbol::Q:
            return 'Q';
         case AminoAcid::Symbol::R:
            return 'R';
         case AminoAcid::Symbol::S:
            return 'S';
         case AminoAcid::Symbol::T:
            return 'T';
         case AminoAcid::Symbol::V:
            return 'V';
         case AminoAcid::Symbol::W:
            return 'W';
         case AminoAcid::Symbol::Y:
            return 'Y';
         case AminoAcid::Symbol::B:
            return 'B';
         case AminoAcid::Symbol::Z:
            return 'Z';
         case AminoAcid::Symbol::X:
            return 'X';
         case AminoAcid::Symbol::STOP:
            return '*';
      }
      SILO_UNREACHABLE();
   }

   static inline std::optional<AminoAcid::Symbol> charToSymbol(char character) {
      switch (character) {
         case '-':
            return AminoAcid::Symbol::GAP;
         case 'A':
         case 'a':
            return AminoAcid::Symbol::A;
         case 'C':
         case 'c':
            return AminoAcid::Symbol::C;
         case 'D':
         case 'd':
            return AminoAcid::Symbol::D;
         case 'E':
         case 'e':
            return AminoAcid::Symbol::E;
         case 'F':
         case 'f':
            return AminoAcid::Symbol::F;
         case 'G':
         case 'g':
            return AminoAcid::Symbol::G;
         case 'H':
         case 'h':
            return AminoAcid::Symbol::H;
         case 'I':
         case 'i':
            return AminoAcid::Symbol::I;
         case 'K':
         case 'k':
            return AminoAcid::Symbol::K;
         case 'L':
         case 'l':
            return AminoAcid::Symbol::L;
         case 'N':
         case 'n':
            return AminoAcid::Symbol::N;
         case 'M':
         case 'm':
            return AminoAcid::Symbol::M;
         case 'P':
         case 'p':
            return AminoAcid::Symbol::P;
         case 'Q':
         case 'q':
            return AminoAcid::Symbol::Q;
         case 'R':
         case 'r':
            return AminoAcid::Symbol::R;
         case 'S':
         case 's':
            return AminoAcid::Symbol::S;
         case 'T':
         case 't':
            return AminoAcid::Symbol::T;
         case 'V':
         case 'v':
            return AminoAcid::Symbol::V;
         case 'W':
         case 'w':
            return AminoAcid::Symbol::W;
         case 'Y':
         case 'y':
            return AminoAcid::Symbol::Y;
         case 'B':
         case 'b':
            return AminoAcid::Symbol::B;
         case 'Z':
         case 'z':
            return AminoAcid::Symbol::Z;
         case 'X':
         case 'x':
            return AminoAcid::Symbol::X;
         case '*':
            return AminoAcid::Symbol::STOP;
         default:
            return std::nullopt;
      }
   }
};

}  // namespace silo
// namespace silo
