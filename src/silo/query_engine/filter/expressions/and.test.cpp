#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createData(const std::string& country, const std::string& date) {
   static std::atomic_int row_id = 0;
   const auto primary_key = row_id++;
   std::string age = row_id % 2 == 0 ? "null" : fmt::format("{}", (3 * row_id) + 4);
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

const QueryTestScenario NESTED_AND = {
   .name = "NESTED_AND",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Details"
  },
  "filterExpression": {
      "children": [
        {
          "column": "date",
          "from": "2009-01-01",
          "to": null,
          "type": "DateBetween"
        },
        {
          "children": [
            {
              "column": "date",
              "from": "2000-01-01",
              "to": null,
              "type": "DateBetween"
            },
            {
              "column": "country",
              "value": "Germany",
              "type": "StringEquals"
            }
          ],
          "type": "And"
        }
      ],
      "type": "And"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"age":13,"country":"Germany","coverage":0.9,"date":"2009-06-07","primaryKey":"id_2"}])"
   )
};

}  // namespace

QUERY_TEST(And, TEST_DATA, ::testing::Values(NESTED_AND));
