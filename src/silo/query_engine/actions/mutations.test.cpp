#include "silo/query_engine/actions/mutations.h"

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

nlohmann::json createDataWithNucleotideSequence(const std::string& nucleotideSequence) {
   random_generator generator;
   const auto primary_key = generator();

   return {
      {"primaryKey", "id_" + to_string(primary_key)},
      {"segment1", {{"sequence", nucleotideSequence}, {"insertions", nlohmann::json::array()}}},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_WITH_ALL_N = createDataWithNucleotideSequence("NNNNN");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithNucleotideSequence("CATTT");

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_ALL_N, DATA_WITH_ALL_MUTATED},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario MUTATIONS = {
   .name = "MUTATIONS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":1,"coverage":3,"mutation":"A1C","mutationFrom":"A","mutationTo":"C","position":1,"proportion":0.3333333333333333,"sequenceName":"segment1"},
{"count":1,"coverage":3,"mutation":"T2A","mutationFrom":"T","mutationTo":"A","position":2,"proportion":0.3333333333333333,"sequenceName":"segment1"},
{"count":1,"coverage":3,"mutation":"G3T","mutationFrom":"G","mutationTo":"T","position":3,"proportion":0.3333333333333333,"sequenceName":"segment1"},
{"count":1,"coverage":3,"mutation":"C4T","mutationFrom":"C","mutationTo":"T","position":4,"proportion":0.3333333333333333,"sequenceName":"segment1"},
{"count":1,"coverage":1,"mutation":"N5T","mutationFrom":"N","mutationTo":"T","position":5,"proportion":1.0,"sequenceName":"segment1"}])"
   )
};

const QueryTestScenario MUTATIONS_SUBFIELDS = {
   .name = "MUTATIONS_SUBFIELDS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "fields": ["count","coverage","mutation"],
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":1,"coverage":3,"mutation":"A1C"},
{"count":1,"coverage":3,"mutation":"T2A"},
{"count":1,"coverage":3,"mutation":"G3T"},
{"count":1,"coverage":3,"mutation":"C4T"},
{"count":1,"coverage":1,"mutation":"N5T"}])"
   )
};

const QueryTestScenario MUTATIONS_SUBFIELDS_HIGH_MIN = {
   .name = "MUTATIONS_SUBFIELDS_HIGH_MIN",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "fields": ["count","coverage","mutation"],
    "minProportion": 0.5
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":1,"coverage":1,"mutation":"N5T"}])"
   )
};

const QueryTestScenario MUTATIONS_INVALID_FIELDS = {
   .name = "MUTATIONS_INVALID_FIELDS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "fields": ["count","foo"],
    "minProportion": 0.5
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_error_message =
      "The attribute 'fields' contains an invalid field 'foo'. Valid fields are mutation, "
      "mutationFrom, mutationTo, position, sequenceName, proportion, coverage, count."
};

const QueryTestScenario MUTATIONS_INVALID_FIELD_TYPE = {
   .name = "MUTATIONS_INVALID_FIELD_TYPE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.5,
    "fields": "count"
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_error_message = "The field 'fields' for a Mutations action must be an array of strings"
};

}  // namespace

QUERY_TEST(
   Mutations,
   TEST_DATA,
   ::testing::Values(
      MUTATIONS,
      MUTATIONS_SUBFIELDS,
      MUTATIONS_SUBFIELDS_HIGH_MIN,
      MUTATIONS_INVALID_FIELDS,
      MUTATIONS_INVALID_FIELD_TYPE
   )
);

namespace {

const QueryTestData TEST_DATA2{
   .ndjson_input_data =
      []() {
         std::vector<nlohmann::json> data;
         data.reserve(100000);
         for (int i = 0; i < 20000; ++i) {
            data.push_back(createDataWithNucleotideSequence("CATTT"));
         }
         for (int i = 0; i < 20000; ++i) {
            data.push_back(createDataWithNucleotideSequence("ATGCN"));
         }
         for (int i = 0; i < 20000; ++i) {
            data.push_back(createDataWithNucleotideSequence("CATTT"));
         }
         for (int i = 0; i < 20000; ++i) {
            data.push_back(createDataWithNucleotideSequence("NNCNN"));
         }
         for (int i = 0; i < 20000; ++i) {
            data.push_back(createDataWithNucleotideSequence("ANCNN"));
         }
         return data;
      }(),
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario MUTATIONS_BIG = {
   .name = "MUTATIONS_BIG",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"count":40000,"coverage":80000,"mutation":"A1C","mutationFrom":"A","mutationTo":"C","position":1,"proportion":0.5,"sequenceName":"segment1"},
{"count":40000,"coverage":60000,"mutation":"T2A","mutationFrom":"T","mutationTo":"A","position":2,"proportion":0.6666666666666666,"sequenceName":"segment1"},
{"count":40000,"coverage":100000,"mutation":"G3C","mutationFrom":"G","mutationTo":"C","position":3,"proportion":0.4,"sequenceName":"segment1"},
{"count":40000,"coverage":100000,"mutation":"G3T","mutationFrom":"G","mutationTo":"T","position":3,"proportion":0.4,"sequenceName":"segment1"},
{"count":40000,"coverage":60000,"mutation":"C4T","mutationFrom":"C","mutationTo":"T","position":4,"proportion":0.6666666666666666,"sequenceName":"segment1"},
{"count":40000,"coverage":40000,"mutation":"N5T","mutationFrom":"N","mutationTo":"T","position":5,"proportion":1.0,"sequenceName":"segment1"}])"
   )
};

const QueryTestScenario MUTATIONS_BIG_SELECTIVE = {
   .name = "MUTATIONS_BIG_SELECTIVE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "NucleotideEquals",
    "position": 3,
    "symbol": "C",
    "sequenceName": "segment1"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"count":40000,"coverage":40000,"mutation":"G3C","mutationFrom":"G","mutationTo":"C","position":3,"proportion":1.0,"sequenceName":"segment1"}
])"
   )
};

const QueryTestScenario MUTATIONS_BIG_SELECTIVE2 = {
   .name = "MUTATIONS_BIG_SELECTIVE2",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "NucleotideEquals",
    "position": 1,
    "symbol": "C",
    "sequenceName": "segment1"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"count":40000,"coverage":40000,"mutation":"A1C","mutationFrom":"A","mutationTo":"C","position":1,"proportion":1.0,"sequenceName":"segment1"},
{"count":40000,"coverage":40000,"mutation":"T2A","mutationFrom":"T","mutationTo":"A","position":2,"proportion":1.0,"sequenceName":"segment1"},
{"count":40000,"coverage":40000,"mutation":"G3T","mutationFrom":"G","mutationTo":"T","position":3,"proportion":1.0,"sequenceName":"segment1"},
{"count":40000,"coverage":40000,"mutation":"C4T","mutationFrom":"C","mutationTo":"T","position":4,"proportion":1.0,"sequenceName":"segment1"},
{"count":40000,"coverage":40000,"mutation":"N5T","mutationFrom":"N","mutationTo":"T","position":5,"proportion":1.0,"sequenceName":"segment1"}
])"
   )
};

const QueryTestScenario MUTATIONS_BIG_SELECTIVE_END = {
   .name = "MUTATIONS_BIG_SELECTIVE_END",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Mutations",
    "minProportion": 0.05
  },
  "filterExpression": {
    "type": "And",
    "children": [
      {
        "type": "NucleotideEquals",
        "position": 1,
        "symbol": "A",
        "sequenceName": "segment1"
      },
      {
        "type": "NucleotideEquals",
        "position": 3,
        "symbol": "C",
        "sequenceName": "segment1"
      }
    ]
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"([
{"count":20000,"coverage":20000,"mutation":"G3C","mutationFrom":"G","mutationTo":"C","position":3,"proportion":1.0,"sequenceName":"segment1"}
])"
   )
};

}  // namespace

QUERY_TEST(
   MutationsBig,
   TEST_DATA2,
   ::testing::Values(
      MUTATIONS_BIG,
      MUTATIONS_BIG_SELECTIVE,
      MUTATIONS_BIG_SELECTIVE2,
      MUTATIONS_BIG_SELECTIVE_END
   )
);
