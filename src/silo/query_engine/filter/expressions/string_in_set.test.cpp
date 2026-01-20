#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(const std::string& primary_key, const std::string& country) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "{}",
   "country": "{}",
   "region": "{}",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": []
   }}
}}
)",
      primary_key,
      country,
      country == "Switzerland" ? "Europe" : (country == "USA" ? "Americas" : "Europe")
   ));
}

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "country"
     type: "string"
   - name: "region"
     type: "string"
     generateIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {createData("id_0", "Switzerland"),
       createData("id_1", "Germany"),
       createData("id_2", "USA"),
       createData("id_3", "Switzerland"),
       createData("id_4", "France"),
       createData("id_5", "Germany")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

const QueryTestScenario STRING_IN_SET_SINGLE_VALUE = {
   .name = "STRING_IN_SET_SINGLE_VALUE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country",
    "values": ["Switzerland"]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0"},{"country":"Switzerland","primaryKey":"id_3"}])"
   )
};

const QueryTestScenario STRING_IN_SET_MULTIPLE_VALUES = {
   .name = "STRING_IN_SET_MULTIPLE_VALUES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country",
    "values": ["Switzerland", "Germany"]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0"},{"country":"Germany","primaryKey":"id_1"},{"country":"Switzerland","primaryKey":"id_3"},{"country":"Germany","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario STRING_IN_SET_NO_MATCH = {
   .name = "STRING_IN_SET_NO_MATCH",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country",
    "values": ["Japan", "China"]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([])")
};

const QueryTestScenario STRING_IN_SET_EMPTY_VALUES = {
   .name = "STRING_IN_SET_EMPTY_VALUES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country",
    "values": []
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(R"([])")
};

const QueryTestScenario STRING_IN_SET_INDEXED_COLUMN = {
   .name = "STRING_IN_SET_INDEXED_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "region"]
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "region",
    "values": ["Europe"]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"id_0","region":"Europe"},{"primaryKey":"id_1","region":"Europe"},{"primaryKey":"id_3","region":"Europe"},{"primaryKey":"id_4","region":"Europe"},{"primaryKey":"id_5","region":"Europe"}])"
   )
};

const QueryTestScenario STRING_IN_SET_WITH_AND = {
   .name = "STRING_IN_SET_WITH_AND",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country", "region"]
  },
  "filterExpression": {
    "type": "And",
    "children": [
      {
        "type": "StringInSet",
        "column": "country",
        "values": ["Switzerland", "Germany", "France"]
      },
      {
        "type": "StringEquals",
        "column": "region",
        "value": "Europe"
      }
    ]
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"Switzerland","primaryKey":"id_0","region":"Europe"},{"country":"Germany","primaryKey":"id_1","region":"Europe"},{"country":"Switzerland","primaryKey":"id_3","region":"Europe"},{"country":"France","primaryKey":"id_4","region":"Europe"},{"country":"Germany","primaryKey":"id_5","region":"Europe"}])"
   )
};

const QueryTestScenario STRING_IN_SET_NEGATED = {
   .name = "STRING_IN_SET_NEGATED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": ["primaryKey", "country"]
  },
  "filterExpression": {
    "type": "Not",
    "child": {
      "type": "StringInSet",
      "column": "country",
      "values": ["Switzerland", "Germany"]
    }
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([{"country":"USA","primaryKey":"id_2"},{"country":"France","primaryKey":"id_4"}])"
   )
};

const QueryTestScenario STRING_IN_SET_MISSING_COLUMN = {
   .name = "STRING_IN_SET_MISSING_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "StringInSet",
    "values": ["Switzerland"]
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'column' is required in a StringInSet expression"
};

const QueryTestScenario STRING_IN_SET_MISSING_VALUES = {
   .name = "STRING_IN_SET_MISSING_VALUES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country"
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'values' is required in a StringInSet expression"
};

const QueryTestScenario STRING_IN_SET_INVALID_COLUMN_TYPE = {
   .name = "STRING_IN_SET_INVALID_COLUMN_TYPE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": 123,
    "values": ["Switzerland"]
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'column' in an StringInSet expression needs to be a string"
};

const QueryTestScenario STRING_IN_SET_INVALID_VALUES_TYPE = {
   .name = "STRING_IN_SET_INVALID_VALUES_TYPE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "country",
    "values": "Switzerland"
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The field 'values' in an StringInSet expression needs to be an array"
};

const QueryTestScenario STRING_IN_SET_NONEXISTENT_COLUMN = {
   .name = "STRING_IN_SET_NONEXISTENT_COLUMN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
    "type": "StringInSet",
    "column": "nonexistent",
    "values": ["Switzerland"]
  }
})"
   ),
   .expected_query_result = {},
   .expected_error_message = "The database does not contain the string column 'nonexistent'"
};

}  // namespace

QUERY_TEST(
   StringInSet,
   TEST_DATA,
   ::testing::Values(
      STRING_IN_SET_SINGLE_VALUE,
      STRING_IN_SET_MULTIPLE_VALUES,
      STRING_IN_SET_NO_MATCH,
      STRING_IN_SET_EMPTY_VALUES,
      STRING_IN_SET_INDEXED_COLUMN,
      STRING_IN_SET_WITH_AND,
      STRING_IN_SET_NEGATED,
      STRING_IN_SET_MISSING_COLUMN,
      STRING_IN_SET_MISSING_VALUES,
      STRING_IN_SET_INVALID_COLUMN_TYPE,
      STRING_IN_SET_INVALID_VALUES_TYPE,
      STRING_IN_SET_NONEXISTENT_COLUMN
   )
);
