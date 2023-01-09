//
// Created by Alexander Taepper on 06.10.22.
//
#include <cassert>
#include <silo/common/silo_symbols.h>

void resolve_alias_test() {
   std::unordered_map<std::string, std::string> alias_key;
   alias_key["X"] = "A";
   alias_key["XY"] = "A.1";

   std::string test_empty;
   std::string test_empty_res = silo::resolve_alias(alias_key, test_empty);
   assert(test_empty_res.empty());

   std::string test1 = "Test";
   std::string test1_res = silo::resolve_alias(alias_key, test1);
   assert(test1_res == "Test");

   std::string test2 = "X";
   std::string test2_res = silo::resolve_alias(alias_key, test2);
   assert(test2_res == "A");

   std::string test3 = "XY";
   std::string test3_res = silo::resolve_alias(alias_key, test3);
   assert(test3_res == "A.1");

   std::string test4 = "X.1.1";
   std::string test4_res = silo::resolve_alias(alias_key, test4);
   assert(test4_res == "A.1.1");

   std::string test5 = "XYX.1.1";
   std::string test5_res = silo::resolve_alias(alias_key, test5);
   assert(test5_res == "XYX.1.1");

   std::string test6 = ".X";
   std::string test6_res = silo::resolve_alias(alias_key, test6);
   assert(test6_res == ".X");
}
