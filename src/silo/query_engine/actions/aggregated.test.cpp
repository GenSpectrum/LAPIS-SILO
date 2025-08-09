#include "silo/query_engine/actions/aggregated.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createData(const std::string& country, const std::string& date) {
   static std::atomic_int id = 0;
   const auto primary_key = id++;
   std::string age = id % 2 == 0 ? "null" : fmt::format("{}", 3 * id + 4);
   float coverage = 0.9;

   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "id_{}",
   "country": "{}",
   "age": {},
   "coverage": {},
   "date": "{}",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": ["2:A"]
   }},
   "unaligned_segment1": "ACGT",
   "gene1": {{
      "sequence": "V",
      "insertions": []
   }}
}}
)",
      primary_key,
      country,
      age,
      coverage,
      date
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
     generateIndex: true
   - name: "age"
     type: "int"
   - name: "coverage"
     type: "float"
   - name: "date"
     type: "date"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {createData("Switzerland", "2020-01-01"),
       createData("Germany", "2000-03-07"),
       createData("Germany", "2009-06-07"),
       createData("Switzerland", "2003-07-02"),
       createData("Switzerland", "2002-01-04"),
       createData("Switzerland", "2001-12-07")},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario COUNT_ALL = {
   .name = "COUNT_ALL",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated"
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count": 6}])"
   )
};

const QueryTestScenario AGGREGATE_ALL = {
   .name = "AGGREGATE_ALL",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "orderByFields": [
      "primaryKey"
    ],
    "groupByFields": [
      "age","country","coverage","date","primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":7,"count":1,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":null,"count":1,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"count":1,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"count":1,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":19,"count": 1,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":null,"count":1,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario AGGREGATE_ALMOST_ALL = {
   .name = "AGGREGATE_ALMOST_ALL",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "orderByFields": [
      "age","date"
    ],
    "groupByFields": [
      "age","country","coverage","date"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[
{"age":null,"count":1,"country":"Germany","coverage":0.9,"date":"2000-03-07"},
{"age":null,"count":1,"country":"Switzerland","coverage":0.9,"date":"2001-12-07"},
{"age":null,"count":1,"country":"Switzerland","coverage":0.9,"date":"2003-07-02"},
{"age":7,"count":1,"country":"Switzerland","coverage":0.9,"date":"2020-01-01"},
{"age":13,"count":1,"country":"Germany","coverage":0.9,"date":"2009-06-07"},
{"age":19,"count": 1,"country":"Switzerland","coverage":0.9,"date":"2002-01-04"}
])"
   )
};

const QueryTestScenario AGGREGATE_SOME = {
   .name = "AGGREGATE_SOME",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "orderByFields": [
      "age", {"field": "count", "order": "descending"}
    ],
    "groupByFields": [
      "age","country","coverage"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":null,"count":2,"country":"Switzerland","coverage":0.9},
{"age":null,"count":1,"country":"Germany","coverage":0.9},
{"age":7,"count":1,"country":"Switzerland","coverage":0.9},
{"age":13,"count":1,"country":"Germany","coverage":0.9},
{"age":19,"count": 1,"country":"Switzerland","coverage":0.9}])"
   )
};

const QueryTestScenario AGGREGATED_LIMIT_OFFSET = {
   .name = "LIMIT_OFFSET",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": [
      "age",
      "country",
      "coverage",
      "date",
      "primaryKey"
    ],
    "orderByFields": [
      "primaryKey"
    ],
    "limit": 3,
    "offset": 1
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":null,"count":1,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"count":1,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"count":1,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"}])"
   )
};

const QueryTestScenario AGGREGATED_LIMIT_WITHOUT_ORDER = {
   .name = "AGGREGATED_LIMIT_WITHOUT_ORDER",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": ["primaryKey"],
    "limit": 1
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message =
      "Offset and limit can only be applied if the output of the operation has some ordering. "
      "Implicit ordering such as in the case of Details/Fasta is also allowed, Aggregated "
      "however produces unordered results."
};

const QueryTestScenario AGGREGATE_UNIQUE = {
   .name = "AGGREGATE_UNIQUE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": [
      "date"
    ],
    "orderByFields": [
      "date"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"date":"2000-03-07","count":1},
{"date":"2001-12-07","count":1},
{"date":"2002-01-04","count":1},
{"date":"2003-07-02","count":1},
{"date":"2009-06-07","count":1},
{"date":"2020-01-01","count":1}])"
   )
};

const QueryTestScenario AGGREGATE_ONE = {
   .name = "AGGREGATE_ONE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": [
      "country"
    ],
    "orderByFields": [
      {"field": "count", "order": "descending"}, "country"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":4,"country":"Switzerland"},
{"count":2,"country":"Germany"}])"
   )
};

const QueryTestScenario AGGREGATE_NULLABLE = {
   .name = "AGGREGATE_NULLABLE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": [
      "age"
    ],
    "orderByFields": [
      "count", {"field": "age", "order": "descending"}
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":19,"count":1},
{"age":13,"count":1},
{"age":7,"count":1},
{"age":null,"count":3}])"
   )
};

const QueryTestScenario DUPLICATE_AGGREGATE = {
   .name = "DUPLICATE_AGGREGATE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Aggregated",
    "groupByFields": [
      "age", "age"
    ],
    "orderByFields": [
      "count", {"field": "age", "order": "descending"}
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":19,"count":1},
{"age":13,"count":1},
{"age":7,"count":1},
{"age":null,"count":3}])"
   )
};

const QueryTestScenario INVALID_GROUP_BY_FIELD_OBJECT = {
   .name = "INVALID_GROUP_BY_FIELD_OBJECT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "groupByFields": [
      {
        "field": "test_boolean_column",
        "order": "ascending"
      }
    ],
    "type": "Aggregated"
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message =
      "{\"field\":\"test_boolean_column\",\"order\":\"ascending\"} is not a valid entry in "
      "groupByFields. Expected type string, got object"
};

const QueryTestScenario INVALID_GROUP_BY_FIELDS = {
   .name = "INVALID_GROUP_BY_FIELDS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "groupByFields": "test_boolean_column",
    "type": "Aggregated"
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message = "groupByFields must be an array"
};

const QueryTestScenario INVALID_ORDER_BY_FIELD_OBJECT = {
   .name = "INVALID_ORDER_BY_FIELD_OBJECT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "orderByFields": [1],
    "type": "Aggregated"
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message =
      "The orderByField '1' must be either a string or an object containing the fields "
      "'field':string and 'order':string, where the value of order is 'ascending' or 'descending'"
};

const QueryTestScenario INVALID_ORDER_BY_FIELDS = {
   .name = "INVALID_ORDER_BY_FIELDS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "orderByFields": "test_boolean_column",
    "type": "Aggregated"
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message = "orderByFields must be an array"
};

}  // namespace

QUERY_TEST(
   Aggregated,
   TEST_DATA,
   ::testing::Values(
      COUNT_ALL,
      AGGREGATE_ALL,
      AGGREGATE_ALMOST_ALL,
      AGGREGATE_SOME,
      AGGREGATED_LIMIT_OFFSET,
      AGGREGATED_LIMIT_WITHOUT_ORDER,
      AGGREGATE_UNIQUE,
      AGGREGATE_ONE,
      AGGREGATE_NULLABLE,
      DUPLICATE_AGGREGATE,
      INVALID_GROUP_BY_FIELD_OBJECT,
      INVALID_GROUP_BY_FIELDS,
      INVALID_ORDER_BY_FIELD_OBJECT,
      INVALID_ORDER_BY_FIELDS
   )
);
