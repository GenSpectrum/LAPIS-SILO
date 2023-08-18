#include "silo/common/nucleotide_symbols.h"

namespace silo {

char Util<NUCLEOTIDE_SYMBOL>::symbolToChar(NUCLEOTIDE_SYMBOL symbol) {
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

std::optional<NUCLEOTIDE_SYMBOL> Util<NUCLEOTIDE_SYMBOL>::charToSymbol(char character) {
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

std::optional<std::vector<NUCLEOTIDE_SYMBOL>> Util<NUCLEOTIDE_SYMBOL>::stringToSymbolVector(
   const std::string& sequence
) {
   const size_t size = sequence.size();
   std::vector<NUCLEOTIDE_SYMBOL> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = Util<NUCLEOTIDE_SYMBOL>::charToSymbol(sequence[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

std::optional<char> Util<NUCLEOTIDE_SYMBOL>::findIllegalChar(const std::string& sequence) {
   for (const auto nuc_char : sequence) {
      auto symbol = Util<NUCLEOTIDE_SYMBOL>::charToSymbol(nuc_char);
      if (symbol == std::nullopt) {
         return nuc_char;
      }
   }
   return std::nullopt;
}

}  // namespace silo