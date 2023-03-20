#include "silo_api/variant_json_serializer.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

struct TestStruct {
   std::string stringField;
   int64_t intField;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TestStruct, stringField, intField);

TEST(variant_json_serializer, deserialize_struct_variant) {
   const int some_number = 42;
   const std::string some_string;
   const nlohmann::json json = {
      {"stringField", some_string},
      {"intField", some_number},
   };

   auto result = json.get<std::variant<TestStruct, std::string>>();

   EXPECT_TRUE(holds_alternative<TestStruct>(result));
   EXPECT_EQ(std::get<TestStruct>(result).stringField, some_string);
   EXPECT_EQ(std::get<TestStruct>(result).intField, some_number);
}

TEST(variant_json_serializer, deserialize_string_variant) {
   const nlohmann::json json = "this is another string";

   auto result = json.get<std::variant<TestStruct, std::string>>();

   EXPECT_TRUE(holds_alternative<std::string>(result));
   EXPECT_EQ(std::get<std::string>(result), "this is another string");
}

TEST(variant_json_serializer, serialize_string_variant) {
   const std::variant<TestStruct, std::string> value = "this is a string";

   auto result = nlohmann::json(value).dump();

   EXPECT_EQ(result, R"("this is a string")");
}

TEST(variant_json_serializer, serialize_struct_variant) {
   const std::variant<TestStruct, std::string> value = TestStruct{"this is another string", 42};

   auto result = nlohmann::json(value).dump();

   EXPECT_EQ(result, R"({"intField":42,"stringField":"this is another string"})");
}
