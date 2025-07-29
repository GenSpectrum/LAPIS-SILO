#include "silo/query_engine/actions/details.h"

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
   "unaligned_segment1": "ACGT",
   "segment1": {{
      "sequence": "ACGT",
      "insertions": ["2:A"]
   }},
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

const QueryTestScenario ALL_DATA = {
   .name = "ALL_DATA",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario LIMIT_OFFSET = {
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

const QueryTestScenario ALL_DATES = {
   .name = "ALL_DATES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": [
      "date", "primaryKey"
    ],
    "orderByFields": [
      "primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"date":"2020-01-01","primaryKey":"id_0"},
{"date":"2000-03-07","primaryKey":"id_1"},
{"date":"2009-06-07","primaryKey":"id_2"},
{"date":"2003-07-02","primaryKey":"id_3"},
{"date":"2002-01-04","primaryKey":"id_4"},
{"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario ALL_DATES_AND_COUNTRIES = {
   .name = "ALL_DATES_AND_COUNTRIES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": [
      "date", "primaryKey", "country"
    ],
    "orderByFields": [
      {"field": "country", "order": "descending"}, "date"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","date":"2001-12-07","primaryKey":"id_5"},
{"country":"Switzerland","date":"2002-01-04","primaryKey":"id_4"},
{"country":"Switzerland","date":"2003-07-02","primaryKey":"id_3"},
{"country":"Switzerland","date":"2020-01-01","primaryKey":"id_0"},
{"country":"Germany","date":"2000-03-07","primaryKey":"id_1"},
{"country":"Germany","date":"2009-06-07","primaryKey":"id_2"}])"
   )
};

const QueryTestScenario DUPLICATE_COUNTRY = {
   .name = "DUPLICATE_COUNTRY",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "fields": [
      "country", "country"
    ],
    "orderByFields": [
      {"field": "country", "order": "descending"}
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland"},
{"country":"Switzerland"},
{"country":"Switzerland"},
{"country":"Switzerland"},
{"country":"Germany"},
{"country":"Germany"}])"
   )
};

const QueryTestScenario LIMIT = {
   .name = "LIMIT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ],
    "limit": 3
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"}])"
   )
};

const QueryTestScenario LIMIT_0 = {
   .name = "LIMIT_0",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ],
    "limit": 0
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_error_message = "If the action contains a limit, it must be a positive number"
};

const QueryTestScenario LIMIT_LARGE = {
   .name = "LIMIT_LARGE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "age", "primaryKey"
    ],
    "limit": 1000
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"},
{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"}])"
   )
};

const QueryTestScenario SINGLE_FIELD_DESCENDING = {
   .name = "SINGLE_FIELD_DESCENDING",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      {"field": "age", "order": "descending"}
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario MULTI_FIELD_SORT = {
   .name = "MULTI_FIELD_SORT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      {"field": "country", "order": "descending"}, "age"
    ],
    "limit": 1000
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"}])"
   )
};

const QueryTestScenario OFFSET = {
   .name = "OFFSET",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ],
    "offset": 3
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario OFFSET_0 = {
   .name = "OFFSET_0",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ],
    "offset": 0
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":7,"country":"Switzerland","coverage":0.9,"date":"2020-01-01","primaryKey":"id_0"},
{"age":null,"country":"Germany","coverage":0.9,"date":"2000-03-07","primaryKey":"id_1"},
{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2003-07-02","primaryKey":"id_3"},
{"age":19,"country":"Switzerland","coverage":0.9,"date":"2002-01-04","primaryKey":"id_4"},
{"age":null,"country":"Switzerland","coverage":0.9,"date":"2001-12-07","primaryKey":"id_5"}])"
   )
};

const QueryTestScenario OFFSET_LARGE = {
   .name = "OFFSET_LARGE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details",
    "orderByFields": [
      "primaryKey"
    ],
    "offset": 123123
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[])"
   )
};

}  // namespace

QUERY_TEST(
   Details,
   TEST_DATA,
   ::testing::Values(
      ALL_DATA,
      LIMIT_OFFSET,
      ALL_DATES,
      ALL_DATES_AND_COUNTRIES,
      DUPLICATE_COUNTRY,
      LIMIT,
      LIMIT_0,
      LIMIT_LARGE,
      SINGLE_FIELD_DESCENDING,
      MULTI_FIELD_SORT,
      OFFSET,
      OFFSET_0,
      OFFSET_LARGE
   )
);
