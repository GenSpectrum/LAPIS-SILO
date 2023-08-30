#include "silo/common/nucleotide_symbols.h"

#include <cassert>

namespace silo {

char Nucleotide::symbolToChar(Nucleotide::Symbol symbol) {
   switch (symbol) {
      case Symbol::GAP:
         return '-';
      case Symbol::A:
         return 'A';
      case Symbol::C:
         return 'C';
      case Symbol::G:
         return 'G';
      case Symbol::T:
         return 'T';
      case Symbol::R:
         return 'R';
      case Symbol::Y:
         return 'Y';
      case Symbol::S:
         return 'S';
      case Symbol::W:
         return 'W';
      case Symbol::K:
         return 'K';
      case Symbol::M:
         return 'M';
      case Symbol::B:
         return 'B';
      case Symbol::D:
         return 'D';
      case Symbol::H:
         return 'H';
      case Symbol::V:
         return 'V';
      case Symbol::N:
         return 'N';
   }
   assert(false);
   return 'N';
}

std::optional<Nucleotide::Symbol> Nucleotide::charToSymbol(char character) {
   switch (character) {
      case '.':
      case '-':
         return Symbol::GAP;
      case 'A':
         return Symbol::A;
      case 'C':
         return Symbol::C;
      case 'G':
         return Symbol::G;
      case 'T':
      case 'U':
         return Symbol::T;
      case 'R':
         return Symbol::R;
      case 'Y':
         return Symbol::Y;
      case 'S':
         return Symbol::S;
      case 'W':
         return Symbol::W;
      case 'K':
         return Symbol::K;
      case 'M':
         return Symbol::M;
      case 'B':
         return Symbol::B;
      case 'D':
         return Symbol::D;
      case 'H':
         return Symbol::H;
      case 'V':
         return Symbol::V;
      case 'N':
         return Symbol::N;
      default:
         return std::nullopt;
   }
}

std::optional<std::vector<Nucleotide::Symbol>> Nucleotide::stringToSymbolVector(
   const std::string& sequence
) {
   const size_t size = sequence.size();
   std::vector<Symbol> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = Nucleotide::charToSymbol(sequence[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

std::optional<char> Nucleotide::findIllegalChar(const std::string& sequence) {
   for (const auto nuc_char : sequence) {
      auto symbol = Nucleotide::charToSymbol(nuc_char);
      if (symbol == std::nullopt) {
         return nuc_char;
      }
   }
   return std::nullopt;
}

}  // namespace silo