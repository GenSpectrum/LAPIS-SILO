#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::optional<bool>& bool_field,
   const std::optional<int32_t>& int_field
) {
   const auto to_json = [](const auto& optional_value) {
      return optional_value.has_value() ? nlohmann::json(optional_value.value())
                                        : nlohmann::json(nullptr);
   };
   nlohmann::json result;
   result["primaryKey"] = primary_key;
   result["boolField"] = to_json(bool_field);
   result["intField"] = to_json(int_field);
   return result;
}

const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "test"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "boolField"
     type: "boolean"
   - name: "intField"
     type: "int"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{{}, {}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("id_true", true, 1),
         createData("id_false", false, 2),
         createData("id_null", std::nullopt, std::nullopt),
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario EQUALS_NON_BOOL_COLUMN_REJECTED = {
   .name = "BOOL_EQUALS_NON_BOOL_COLUMN_REJECTED",
   .query = "default.filter(intField = true).project(primaryKey)",
   .expected_error_message = "The column 'intField' is not of type bool"
};

const QueryTestScenario NOT_EQUALS_NON_BOOL_COLUMN_REJECTED = {
   .name = "BOOL_NOT_EQUALS_NON_BOOL_COLUMN_REJECTED",
   .query = "default.filter(intField <> true).project(primaryKey)",
   .expected_error_message = "The column 'intField' is not of type bool"
};

const QueryTestScenario EQUALS_TRUE = {
   .name = "BOOL_EQUALS_TRUE",
   .query = "default.filter(boolField = true).project(primaryKey)",
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_true"}])")
};

const QueryTestScenario EQUALS_FALSE = {
   .name = "BOOL_EQUALS_FALSE",
   .query = "default.filter(boolField = false).project(primaryKey)",
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_false"}])")
};

// The column may sit on either side.
const QueryTestScenario EQUALS_COLUMN_ON_RIGHT = {
   .name = "BOOL_EQUALS_COLUMN_ON_RIGHT",
   .query = "default.filter(true = boolField).project(primaryKey)",
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_true"}])")
};

}  // namespace

QUERY_TEST(
   ComparisonBool,
   TEST_DATA,
   ::testing::Values(
      EQUALS_NON_BOOL_COLUMN_REJECTED,
      NOT_EQUALS_NON_BOOL_COLUMN_REJECTED,
      EQUALS_TRUE,
      EQUALS_FALSE,
      EQUALS_COLUMN_ON_RIGHT
   )
);
