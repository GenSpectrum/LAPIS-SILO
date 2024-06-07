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

const silo::SymbolMap<Nucleotide, std::vector<Nucleotide::Symbol>> Nucleotide::AMBIGUITY_SYMBOLS{{{
   {Nucleotide::Symbol::GAP},
   {Nucleotide::Symbol::A,
    Nucleotide::Symbol::R,
    Nucleotide::Symbol::M,
    Nucleotide::Symbol::W,
    Nucleotide::Symbol::D,
    Nucleotide::Symbol::H,
    Nucleotide::Symbol::V,
    Nucleotide::Symbol::N},
   {Nucleotide::Symbol::C,
    Nucleotide::Symbol::Y,
    Nucleotide::Symbol::M,
    Nucleotide::Symbol::S,
    Nucleotide::Symbol::B,
    Nucleotide::Symbol::H,
    Nucleotide::Symbol::V,
    Nucleotide::Symbol::N},
   {Nucleotide::Symbol::G,
    Nucleotide::Symbol::R,
    Nucleotide::Symbol::K,
    Nucleotide::Symbol::S,
    Nucleotide::Symbol::B,
    Nucleotide::Symbol::D,
    Nucleotide::Symbol::V,
    Nucleotide::Symbol::N},
   {Nucleotide::Symbol::T,
    Nucleotide::Symbol::Y,
    Nucleotide::Symbol::K,
    Nucleotide::Symbol::W,
    Nucleotide::Symbol::B,
    Nucleotide::Symbol::D,
    Nucleotide::Symbol::H,
    Nucleotide::Symbol::N},
   {Nucleotide::Symbol::R},
   {Nucleotide::Symbol::Y},
   {Nucleotide::Symbol::S},
   {Nucleotide::Symbol::W},
   {Nucleotide::Symbol::K},
   {Nucleotide::Symbol::M},
   {Nucleotide::Symbol::B},
   {Nucleotide::Symbol::D},
   {Nucleotide::Symbol::H},
   {Nucleotide::Symbol::V},
   {Nucleotide::Symbol::N},
}}};

std::string Nucleotide::suffixWith(std::string_view suffix) {
   return std::string(SYMBOL_NAME_SHORT).append(suffix);
}

}  // namespace silo