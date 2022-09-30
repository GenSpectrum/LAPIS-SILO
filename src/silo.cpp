//
// Created by Alexander Taepper on 27.09.22.
//

#include "silo/silo.h"

silo::Symbol silo::to_symbol(char c) {
   Symbol s = Symbol::gap;
   switch (c) {
      case '.':
      case '-':
         s = Symbol::gap;
         break;
      case 'A':
         s = Symbol::A;
         break;
      case 'C':
         s = Symbol::C;
         break;
      case 'G':
         s = Symbol::G;
         break;
      case 'T':
      case 'U':
         s = Symbol::T;
         break;
      case 'R':
         s = Symbol::R;
         break;
      case 'Y':
         s = Symbol::Y;
         break;
      case 'S':
         s = Symbol::S;
         break;
      case 'W':
         s = Symbol::W;
         break;
      case 'K':
         s = Symbol::K;
         break;
      case 'M':
         s = Symbol::M;
         break;
      case 'B':
         s = Symbol::B;
         break;
      case 'D':
         s = Symbol::D;
         break;
      case 'H':
         s = Symbol::H;
         break;
      case 'V':
         s = Symbol::V;
         break;
      case 'N':
         s = Symbol::N;
         break;
      default:
         std::cerr << "unrecognized symbol " << c << std::endl;
   }
   return s;
}

std::string silo::getPangoPrefix(const std::string& pango_lineage) {
   std::string pangoPref;
   if (pango_lineage.size() > 2) {
      std::stringstream ss(pango_lineage);
      if (!getline(ss, pangoPref, '.')) {
         std::cerr << "Non-covered case of pango lineage!" << std::endl;
         return "Not-recognized";
      }
   } else {
      pangoPref = pango_lineage;
   }
   return pangoPref;
}
