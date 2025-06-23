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
   "metadata": {{
      "primaryKey": "id_{}",
      "country": "{}",
      "age": {},
      "coverage": {},
      "date": "{}"
   }},
   "alignedNucleotideSequences": {{
      "segment1": "ACGT"
   }},
   "unalignedNucleotideSequences": {{
      "segment1": "ACGT"
   }},
   "nucleotideInsertions": {{
      "segment1": ["2:A"]
   }},
   "alignedAminoAcidSequences": {{
      "gene1": "V"
   }},
   "aminoAcidInsertions": {{
      "gene1": []
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
    "type": "Details",
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
[{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"}])"
   )
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
      AGGREGATE_UNIQUE,
      AGGREGATE_ONE,
      AGGREGATE_NULLABLE
   )
);
