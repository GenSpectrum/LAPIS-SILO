//
// Created by Alexander Taepper on 30.09.22.
//
#include "query_test.cpp"
#include "resolve_alias_test.cpp"
#include "silo/silo.h"

int main(int argc, char* argv[]) {
   if (argc < 2) {
      std::cerr << "No test specified" << std::endl;
      return -1;
   }
   std::string arg(argv[1]);
   if (arg == "resolve_alias") {
      resolve_alias_test();
   } else if (arg == "pango_util") {
   } else {
      std::cerr << "Unknown Test. " << arg << std::endl;
      return -1;
   }
}

int limit_meta_to_seqs() {
   return -1;
}
