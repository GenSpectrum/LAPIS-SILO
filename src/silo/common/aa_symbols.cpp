#include "silo/common/aa_symbols.h"

#include <cassert>

char silo::AminoAcid::symbolToChar(silo::AminoAcid::Symbol symbol) {
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
   assert(false);
   return 'X';
}

std::optional<silo::AminoAcid::Symbol> silo::AminoAcid::charToSymbol(char character) {
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

std::optional<std::vector<silo::AminoAcid::Symbol>> silo::AminoAcid::stringToSymbolVector(
   const std::string& sequence
) {
   const size_t size = sequence.size();
   std::vector<silo::AminoAcid::Symbol> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = silo::AminoAcid::charToSymbol(sequence[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

std::optional<char> silo::AminoAcid::findIllegalChar(const std::string& sequence) {
   for (const auto aa_char : sequence) {
      auto symbol = silo::AminoAcid::charToSymbol(aa_char);
      if (symbol == std::nullopt) {
         return aa_char;
      }
   }
   return std::nullopt;
}
