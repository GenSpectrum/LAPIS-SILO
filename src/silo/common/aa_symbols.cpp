#include "silo/common/aa_symbols.h"

char silo::Util<silo::AA_SYMBOL>::symbolToChar(silo::AA_SYMBOL symbol) {
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

std::optional<silo::AA_SYMBOL> silo::Util<silo::AA_SYMBOL>::charToSymbol(char character) {
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

std::optional<std::vector<silo::AA_SYMBOL>> silo::Util<silo::AA_SYMBOL>::stringToSymbolVector(
   const std::string& nucleotides
) {
   const size_t size = nucleotides.size();
   std::vector<silo::AA_SYMBOL> result(size);
   for (size_t i = 0; i < size; ++i) {
      auto symbol = Util<silo::AA_SYMBOL>::charToSymbol(nucleotides[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result[i] = *symbol;
   }
   return result;
}

std::optional<char> silo::Util<silo::AA_SYMBOL>::findIllegalChar(const std::string& sequence) {
   for (const auto aa_char : sequence) {
      auto symbol = Util<silo::AA_SYMBOL>::charToSymbol(aa_char);
      if (symbol == std::nullopt) {
         return aa_char;
      }
   }
   return std::nullopt;
}
