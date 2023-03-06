#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"
#include <variant>
#include <string>
#include "silo/variant_json_serializer.h"

struct test_struct {
   std::string stringField;
   int64_t intField;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(test_struct, stringField, intField);

TEST(variant_json_serializer, deserialize_struct_variant) {
   nlohmann::json json = {
      {"stringField", "this is a string"},
      {"intField", 42},
   };

   auto result = json.get<std::variant<test_struct, std::string>>();

   EXPECT_TRUE(holds_alternative<test_struct>(result));
   EXPECT_EQ(std::get<test_struct>(result).stringField, "this is a string");
   EXPECT_EQ(std::get<test_struct>(result).intField, 42);
}

TEST(variant_json_serializer, deserialize_string_variant) {
   nlohmann::json json = "this is another string";

   auto result = json.get<std::variant<test_struct, std::string>>();

   EXPECT_TRUE(holds_alternative<std::string>(result));
   EXPECT_EQ(std::get<std::string>(result), "this is another string");
}

TEST(variant_json_serializer, serialize_string_variant) {
   std::variant<test_struct, std::string> value = "this is a string";

   auto result = nlohmann::json(value).dump();

   EXPECT_EQ(result, R"("this is a string")");
}

TEST(variant_json_serializer, serialize_struct_variant) {
   std::variant<test_struct, std::string> value = test_struct{"this is another string", 42};

   auto result = nlohmann::json(value).dump();

   EXPECT_EQ(result, R"({"intField":42,"stringField":"this is another string"})");
}
