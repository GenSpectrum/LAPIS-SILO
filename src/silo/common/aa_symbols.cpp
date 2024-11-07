#include "silo/common/aa_symbols.h"

#include <cstddef>
#include <string>

#include "silo/common/panic.h"
#include "silo/common/symbol_map.h"

namespace silo {

char AminoAcid::symbolToChar(AminoAcid::Symbol symbol) {
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

std::optional<AminoAcid::Symbol> AminoAcid::charToSymbol(char character) {
   switch (character) {
      case '-':
         return AminoAcid::Symbol::GAP;
      case 'A':
         return AminoAcid::Symbol::A;
      case 'C':
         return AminoAcid::Symbol::C;
      case 'D':
         return AminoAcid::Symbol::D;
      case 'E':
         return AminoAcid::Symbol::E;
      case 'F':
         return AminoAcid::Symbol::F;
      case 'G':
         return AminoAcid::Symbol::G;
      case 'H':
         return AminoAcid::Symbol::H;
      case 'I':
         return AminoAcid::Symbol::I;
      case 'K':
         return AminoAcid::Symbol::K;
      case 'L':
         return AminoAcid::Symbol::L;
      case 'N':
         return AminoAcid::Symbol::N;
      case 'M':
         return AminoAcid::Symbol::M;
      case 'P':
         return AminoAcid::Symbol::P;
      case 'Q':
         return AminoAcid::Symbol::Q;
      case 'R':
         return AminoAcid::Symbol::R;
      case 'S':
         return AminoAcid::Symbol::S;
      case 'T':
         return AminoAcid::Symbol::T;
      case 'V':
         return AminoAcid::Symbol::V;
      case 'W':
         return AminoAcid::Symbol::W;
      case 'Y':
         return AminoAcid::Symbol::Y;
      case 'B':
         return AminoAcid::Symbol::B;
      case 'Z':
         return AminoAcid::Symbol::Z;
      case 'X':
         return AminoAcid::Symbol::X;
      case '*':
         return AminoAcid::Symbol::STOP;
      default:
         return std::nullopt;
   }
}

std::optional<std::vector<AminoAcid::Symbol>> AminoAcid::stringToSymbolVector(
   const std::string& sequence
) {
   const size_t size = sequence.size();
   std::vector<AminoAcid::Symbol> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = AminoAcid::charToSymbol(sequence[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

std::optional<char> AminoAcid::findIllegalChar(const std::string& sequence) {
   for (const auto aa_char : sequence) {
      auto symbol = AminoAcid::charToSymbol(aa_char);
      if (symbol == std::nullopt) {
         return aa_char;
      }
   }
   return std::nullopt;
}

const silo::SymbolMap<AminoAcid, std::vector<AminoAcid::Symbol>> AminoAcid::AMBIGUITY_SYMBOLS{{{
   {AminoAcid::Symbol::GAP, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::A, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::C, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::D, AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::E, AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::F, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::G, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::H, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::I, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::K, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::L, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::M, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::N, AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::P, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Q, AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::R, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::S, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::T, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::V, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::W, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Y, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::STOP, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::X},
}}};
}  // namespace silo
