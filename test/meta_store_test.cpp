//
// Created by Alexander Taepper on 06.10.22.
//
#include "silo/meta_store.h"

void resolve_alias_test() {
   std::unordered_map<std::string, std::string> alias_key;
   alias_key["X"] = "A";
   alias_key["XY"] = "A.1";

   std::string test_empty;
   silo::resolve_alias(alias_key, test_empty);
   assert(test_empty.empty());

   std::string test1 = "Test";
   silo::resolve_alias(alias_key, test1);
   assert(test1 == "Test");

   std::string test2 = "X";
   silo::resolve_alias(alias_key, test2);
   assert(test2 == "A");

   std::string test3 = "XY";
   silo::resolve_alias(alias_key, test3);
   assert(test3 == "A.1");

   std::string test4 = "X.1.1";
   silo::resolve_alias(alias_key, test4);
   assert(test4 == "A.1.1");

   std::string test5 = "XYX.1.1";
   silo::resolve_alias(alias_key, test5);
   assert(test5 == "XYX.1.1");

   std::string test6 = ".X";
   silo::resolve_alias(alias_key, test6);
   assert(test6 == ".X");
}
