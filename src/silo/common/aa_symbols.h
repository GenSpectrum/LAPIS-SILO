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
      O,     // Pyrrolysine
      P,     // Proline
      Q,     // Glutamine
      R,     // Arginine
      S,     // Serine
      T,     // Threonine
      U,     // Selenocysteine
      V,     // Valine
      W,     // Tryptophan
      Y,     // Tyrosine
      B,     // Aspartic acid or Asparagine
      J,     // Leucine or Isoleucine
      Z,     // Glutamine or Glutamic acid
      STOP,  // Stop codon
      X,     // Any amino acid
   };

   static constexpr schema::ColumnType COLUMN_TYPE = schema::ColumnType::AMINO_ACID_SEQUENCE;
   using Column = storage::column::SequenceColumnPartition<AminoAcid>;

   static constexpr uint32_t COUNT = 28;
   static_assert(COUNT == static_cast<uint32_t>(Symbol::X) + 1);

   static constexpr std::string_view SYMBOL_NAME = "AminoAcid";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "amino acid";
   static constexpr std::string_view PREFIX = "aa_";

   static constexpr std::array<Symbol, COUNT> SYMBOLS{
      Symbol::GAP, Symbol::A, Symbol::C, Symbol::D, Symbol::E, Symbol::F,    Symbol::G,
      Symbol::H,   Symbol::I, Symbol::K, Symbol::L, Symbol::M, Symbol::N,    Symbol::O,
      Symbol::P,   Symbol::Q, Symbol::R, Symbol::S, Symbol::T, Symbol::U,    Symbol::V,
      Symbol::W,   Symbol::Y, Symbol::B, Symbol::J, Symbol::Z, Symbol::STOP, Symbol::X,
   };

   static constexpr std::array<Symbol, 24> VALID_MUTATION_SYMBOLS{
      Symbol::GAP,   // Gap, deletion in sequence
      Symbol::A,     // Alanine
      Symbol::C,     // Cysteine
      Symbol::D,     // Aspartic Acid
      Symbol::E,     // Glutamic Acid
      Symbol::F,     // Phenylalanine
      Symbol::G,     // Glycine
      Symbol::H,     // Histidine
      Symbol::I,     // Isoleucine
      Symbol::K,     // Lysine
      Symbol::L,     // Leucine
      Symbol::M,     // Methionine
      Symbol::N,     // Asparagine
      Symbol::O,     // Pyrrolysine
      Symbol::P,     // Proline
      Symbol::Q,     // Glutamine
      Symbol::R,     // Arginine
      Symbol::S,     // Serine
      Symbol::T,     // Threonine
      Symbol::U,     // Selenocysteine
      Symbol::V,     // Valine
      Symbol::W,     // Tryptophan
      Symbol::Y,     // Tyrosine
      Symbol::STOP,  // Stop codon, Star-character in sequence
   };

   static constexpr std::array<Symbol, 4> INVALID_MUTATION_SYMBOLS{
      Symbol::B,  // Aspartic acid or Asparagine
      Symbol::J,  // Leucine or Isoleucine
      Symbol::Z,  // Glutamine or Glutamic acid
      Symbol::X,  // Any amino acid
   };

   static_assert(INVALID_MUTATION_SYMBOLS.size() + VALID_MUTATION_SYMBOLS.size() == SYMBOLS.size());

   static const silo::SymbolMap<AminoAcid, std::vector<AminoAcid::Symbol>> AMBIGUITY_SYMBOLS;

   static constexpr Symbol SYMBOL_MISSING = Symbol::X;

   static inline char symbolToChar(AminoAcid::Symbol symbol) {
      switch (symbol) {
         case Symbol::GAP:
            return '-';
         case Symbol::A:
            return 'A';
         case Symbol::C:
            return 'C';
         case Symbol::D:
            return 'D';
         case Symbol::E:
            return 'E';
         case Symbol::F:
            return 'F';
         case Symbol::G:
            return 'G';
         case Symbol::H:
            return 'H';
         case Symbol::I:
            return 'I';
         case Symbol::K:
            return 'K';
         case Symbol::L:
            return 'L';
         case Symbol::N:
            return 'N';
         case Symbol::M:
            return 'M';
         case Symbol::O:
            return 'O';
         case Symbol::P:
            return 'P';
         case Symbol::Q:
            return 'Q';
         case Symbol::R:
            return 'R';
         case Symbol::S:
            return 'S';
         case Symbol::T:
            return 'T';
         case Symbol::U:
            return 'U';
         case Symbol::V:
            return 'V';
         case Symbol::W:
            return 'W';
         case Symbol::Y:
            return 'Y';
         case Symbol::B:
            return 'B';
         case Symbol::J:
            return 'J';
         case Symbol::Z:
            return 'Z';
         case Symbol::X:
            return 'X';
         case Symbol::STOP:
            return '*';
      }
      SILO_UNREACHABLE();
   }

   static inline std::optional<AminoAcid::Symbol> charToSymbol(char character) {
      switch (character) {
         case '-':
            return Symbol::GAP;
         case 'A':
         case 'a':
            return Symbol::A;
         case 'C':
         case 'c':
            return Symbol::C;
         case 'D':
         case 'd':
            return Symbol::D;
         case 'E':
         case 'e':
            return Symbol::E;
         case 'F':
         case 'f':
            return Symbol::F;
         case 'G':
         case 'g':
            return Symbol::G;
         case 'H':
         case 'h':
            return Symbol::H;
         case 'I':
         case 'i':
            return Symbol::I;
         case 'K':
         case 'k':
            return Symbol::K;
         case 'L':
         case 'l':
            return Symbol::L;
         case 'M':
         case 'm':
            return Symbol::M;
         case 'N':
         case 'n':
            return Symbol::N;
         case 'O':
         case 'o':
            return Symbol::O;
         case 'P':
         case 'p':
            return Symbol::P;
         case 'Q':
         case 'q':
            return Symbol::Q;
         case 'R':
         case 'r':
            return Symbol::R;
         case 'S':
         case 's':
            return Symbol::S;
         case 'T':
         case 't':
            return Symbol::T;
         case 'U':
         case 'u':
            return Symbol::U;
         case 'V':
         case 'v':
            return Symbol::V;
         case 'W':
         case 'w':
            return Symbol::W;
         case 'Y':
         case 'y':
            return Symbol::Y;
         case 'B':
         case 'b':
            return Symbol::B;
         case 'J':
         case 'j':
            return Symbol::J;
         case 'Z':
         case 'z':
            return Symbol::Z;
         case 'X':
         case 'x':
            return Symbol::X;
         case '*':
            return Symbol::STOP;
         default:
            return std::nullopt;
      }
   }
};

}  // namespace silo
// namespace silo
