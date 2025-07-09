#include "silo/query_engine/actions/insertions.h"

#include <fmt/format.h>
#include <fmt/ranges.h>
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

nlohmann::json createData(
   std::vector<std::string> insertions,
   std::vector<std::string> aa_insertions
) {
   static std::atomic_int id = 0;
   const auto primary_key = id++;

   std::string country = id % 3 == 0 ? "Germany" : "Switzerland";

   for (auto& insertion : insertions) {
      insertion = fmt::format("\"{}\"", insertion);
   }
   for (auto& insertion : aa_insertions) {
      insertion = fmt::format("\"{}\"", insertion);
   }

   return nlohmann::json::parse(fmt::format(
      R"(
{{
   "primaryKey": "id_{}",
   "country": "{}",
   "unaligned_segment1": null,
   "segment1": {{
      "seq": "",
      "insertions": [{}]
   }},
   "gene1": {{
      "seq": "",
      "insertions": [{}]
   }}
}}
)",
      primary_key,
      country,
      fmt::join(insertions, ", "),
      fmt::join(aa_insertions, ", ")
   ));
}

const nlohmann::json DATA_1 = createData({"123:ATGCN"}, {"123:AY"});
const nlohmann::json DATA_2 = createData({"123:ATGCN"}, {"123:AY"});
const nlohmann::json DATA_3 = createData({"123:NNNNNNNN"}, {"123:XXX"});
const nlohmann::json DATA_4 = createData({"1:CCC"}, {"1:A"});
const nlohmann::json DATA_5 = createData({"123:ATGCN"}, {"123:AY"});

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
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA_1, DATA_2, DATA_3, DATA_4, DATA_5},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario INSERTIONS = {
   .name = "INSERTIONS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Insertions",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      "insertion"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":3,"insertedSymbols":"ATGCN","insertion":"ins_123:ATGCN","position":123,"sequenceName":"segment1"},
{"count":1,"insertedSymbols":"NNNNNNNN","insertion":"ins_123:NNNNNNNN","position":123,"sequenceName":"segment1"},
{"count":1,"insertedSymbols":"CCC","insertion":"ins_1:CCC","position":1,"sequenceName":"segment1"}])"
   )
};

const QueryTestScenario INSERTIONS_NO_SEQUENCE_NAMES = {
   .name = "INSERTIONS_NO_SEQUENCE_NAMES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Insertions",
    "orderByFields": ["insertion"]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":3,"insertedSymbols":"ATGCN","insertion":"ins_123:ATGCN","position":123,"sequenceName":"segment1"},
{"count":1,"insertedSymbols":"NNNNNNNN","insertion":"ins_123:NNNNNNNN","position":123,"sequenceName":"segment1"},
{"count":1,"insertedSymbols":"CCC","insertion":"ins_1:CCC","position":1,"sequenceName":"segment1"}])"
   )
};

const QueryTestScenario INSERTIONS_SEQUENCE_NAME_NOT_IN_DATABASE = {
   .name = "INSERTIONS_SEQUENCE_NAME_NOT_IN_DATABASE",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "Insertions",
    "sequenceNames": [
      "not_in_database"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_error_message =
      "The database does not contain the Nucleotide sequence 'not_in_database'"
};

const QueryTestScenario AA_INSERTIONS_ALL = {
   .name = "AA_INSERTIONS_ALL",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "AminoAcidInsertions",
    "orderByFields": ["insertion"]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":3,"insertedSymbols":"AY","insertion":"ins_123:AY","position":123,"sequenceName":"gene1"},
{"count":1,"insertedSymbols":"XXX","insertion":"ins_123:XXX","position":123,"sequenceName":"gene1"},
{"count":1,"insertedSymbols":"A","insertion":"ins_1:A","position":1,"sequenceName":"gene1"}])"
   )
};

const QueryTestScenario AA_INSERTIONS_SUBSET = {
   .name = "AA_INSERTIONS_SUBSET",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "AminoAcidInsertions",
    "orderByFields": ["insertion"]
  },
  "filterExpression": {
    "type": "StringEquals",
    "column": "country",
    "value": "Switzerland"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"count":3,"insertedSymbols":"AY","insertion":"ins_123:AY","position":123,"sequenceName":"gene1"},
{"count":1,"insertedSymbols":"A","insertion":"ins_1:A","position":1,"sequenceName":"gene1"}])"
   )
};

}  // namespace

QUERY_TEST(
   Insertions,
   TEST_DATA,
   ::testing::Values(
      INSERTIONS,
      INSERTIONS_NO_SEQUENCE_NAMES,
      INSERTIONS_SEQUENCE_NAME_NOT_IN_DATABASE,
      AA_INSERTIONS_ALL,
      AA_INSERTIONS_SUBSET
   )
);
