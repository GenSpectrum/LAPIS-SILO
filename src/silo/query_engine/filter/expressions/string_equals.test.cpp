#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::optional<std::string>& string_field,
   const std::optional<std::string>& indexed_string_field
) {
   nlohmann::json result;
   result["primaryKey"] = primary_key;
   result["stringField"] =
      string_field.has_value() ? nlohmann::json(string_field.value()) : nlohmann::json(nullptr);
   result["indexedStringField"] = indexed_string_field.has_value()
                                     ? nlohmann::json(indexed_string_field.value())
                                     : nlohmann::json(nullptr);
   return result;
}

const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "test"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "stringField"
     type: "string"
   - name: "indexedStringField"
     type: "string"
     generateIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{{}, {}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("id_0", "value1", "indexed1"),
         createData("id_1", std::nullopt, "indexed2"),
         createData("id_2", "value2", std::nullopt),
         createData("id_3", "value3", "indexed3"),
         createData("id_4", std::nullopt, std::nullopt),
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

// Tests for StringEquals with value: null (should rewrite to IsNull)
const QueryTestScenario STRING_EQUALS_NULL_STRING_COLUMN = {
   .name = "STRING_EQUALS_NULL_STRING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "stringField",
    "value": null
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_1"},{"primaryKey":"id_4"}])")
};

const QueryTestScenario STRING_EQUALS_NULL_INDEXED_STRING_COLUMN = {
   .name = "STRING_EQUALS_NULL_INDEXED_STRING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "indexedStringField",
    "value": null
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_2"},{"primaryKey":"id_4"}])")
};

const QueryTestScenario STRING_EQUALS_NULL_NEGATED = {
   .name = "STRING_EQUALS_NULL_NEGATED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "Not",
    "child": {
      "type": "StringEquals",
      "column": "stringField",
      "value": null
    }
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_0"},{"primaryKey":"id_2"},{"primaryKey":"id_3"}])"
      )
};

const QueryTestScenario STRING_EQUALS_VALUE = {
   .name = "STRING_EQUALS_VALUE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "stringField",
    "value": "value1"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_0"}])")
};

const QueryTestScenario STRING_EQUALS_INDEXED_VALUE = {
   .name = "STRING_EQUALS_INDEXED_VALUE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "indexedStringField",
    "value": "indexed1"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_0"}])")
};

const QueryTestScenario STRING_EQUALS_NO_MATCH = {
   .name = "STRING_EQUALS_NO_MATCH",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "stringField",
    "value": "nonexistent"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([])")
};

}  // namespace

QUERY_TEST(
   StringEquals,
   TEST_DATA,
   ::testing::Values(
      STRING_EQUALS_NULL_STRING_COLUMN,
      STRING_EQUALS_NULL_INDEXED_STRING_COLUMN,
      STRING_EQUALS_NULL_NEGATED,
      STRING_EQUALS_VALUE,
      STRING_EQUALS_INDEXED_VALUE,
      STRING_EQUALS_NO_MATCH
   )
);
