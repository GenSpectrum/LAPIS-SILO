#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::optional<std::string>& string_field,
   const std::optional<std::string>& indexed_string_field,
   const std::optional<int>& int_field,
   const std::optional<double>& float_field,
   const std::optional<bool>& bool_field,
   const std::optional<std::string>& date_field
) {
   nlohmann::json result;
   result["primaryKey"] = primary_key;
   result["stringField"] =
      string_field.has_value() ? nlohmann::json(string_field.value()) : nlohmann::json(nullptr);
   result["indexedStringField"] = indexed_string_field.has_value()
                                     ? nlohmann::json(indexed_string_field.value())
                                     : nlohmann::json(nullptr);
   result["intField"] =
      int_field.has_value() ? nlohmann::json(int_field.value()) : nlohmann::json(nullptr);
   result["floatField"] =
      float_field.has_value() ? nlohmann::json(float_field.value()) : nlohmann::json(nullptr);
   result["boolField"] =
      bool_field.has_value() ? nlohmann::json(bool_field.value()) : nlohmann::json(nullptr);
   result["dateField"] =
      date_field.has_value() ? nlohmann::json(date_field.value()) : nlohmann::json(nullptr);
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
   - name: "intField"
     type: "int"
   - name: "floatField"
     type: "float"
   - name: "boolField"
     type: "boolean"
   - name: "dateField"
     type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{{}, {}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("id_0", "value1", "indexed1", 10, 1.5, true, "2024-01-01"),
         createData("id_1", std::nullopt, "indexed2", 20, 2.5, false, "2024-01-02"),
         createData("id_2", "value2", std::nullopt, 30, 3.5, true, "2024-01-03"),
         createData("id_3", "value3", "indexed3", std::nullopt, 4.5, false, "2024-01-04"),
         createData("id_4", "value4", "indexed4", 50, std::nullopt, true, "2024-01-05"),
         createData("id_5", "value5", "indexed5", 60, 6.5, std::nullopt, "2024-01-06"),
         createData("id_6", "value6", "indexed6", 70, 7.5, false, std::nullopt),
         createData(
            "id_7",
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt
         ),
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario IS_NULL_STRING_COLUMN = {
   .name = "IS_NULL_STRING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "stringField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_1"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_INDEXED_STRING_COLUMN = {
   .name = "IS_NULL_INDEXED_STRING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "indexedStringField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_2"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_INT_COLUMN = {
   .name = "IS_NULL_INT_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "intField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_3"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_FLOAT_COLUMN = {
   .name = "IS_NULL_FLOAT_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "floatField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_4"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_BOOL_COLUMN = {
   .name = "IS_NULL_BOOL_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "boolField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_5"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_DATE_COLUMN = {
   .name = "IS_NULL_DATE_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "dateField"
  }
})"
   ),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"id_6"},{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_NEGATED = {
   .name = "IS_NULL_NEGATED",
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
      "type": "IsNull",
      "column": "stringField"
    }
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"id_0"},{"primaryKey":"id_2"},{"primaryKey":"id_3"},{"primaryKey":"id_4"},{"primaryKey":"id_5"},{"primaryKey":"id_6"}])"
   )
};

const QueryTestScenario IS_NOT_NULL = {
   .name = "IS_NOT_NULL",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "IsNotNull",
    "column": "stringField"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"id_0"},{"primaryKey":"id_2"},{"primaryKey":"id_3"},{"primaryKey":"id_4"},{"primaryKey":"id_5"},{"primaryKey":"id_6"}])"
   )
};

const QueryTestScenario IS_NULL_WITH_AND = {
   .name = "IS_NULL_WITH_AND",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey"]
  },
  "filterExpression": {
    "type": "And",
    "children": [
      {
        "type": "IsNull",
        "column": "stringField"
      },
      {
        "type": "IsNull",
        "column": "intField"
      }
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"id_7"}])")
};

const QueryTestScenario IS_NULL_MISSING_COLUMN = {
   .name = "IS_NULL_MISSING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "IsNull"
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'column' is required in an IsNull expression"
};

const QueryTestScenario IS_NULL_INVALID_COLUMN_TYPE = {
   .name = "IS_NULL_INVALID_COLUMN_TYPE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "IsNull",
    "column": 123
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'column' in an IsNull expression must be a string"
};

const QueryTestScenario IS_NULL_NONEXISTENT_COLUMN = {
   .name = "IS_NULL_NONEXISTENT_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "IsNull",
    "column": "nonexistent"
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message =
      "The database does not contain a column 'nonexistent' that supports null checks"
};

}  // namespace

QUERY_TEST(
   IsNull,
   TEST_DATA,
   ::testing::Values(
      IS_NULL_STRING_COLUMN,
      IS_NULL_INDEXED_STRING_COLUMN,
      IS_NULL_INT_COLUMN,
      IS_NULL_FLOAT_COLUMN,
      IS_NULL_BOOL_COLUMN,
      IS_NULL_DATE_COLUMN,
      IS_NULL_NEGATED,
      IS_NOT_NULL,
      IS_NULL_WITH_AND,
      IS_NULL_MISSING_COLUMN,
      IS_NULL_INVALID_COLUMN_TYPE,
      IS_NULL_NONEXISTENT_COLUMN
   )
);
