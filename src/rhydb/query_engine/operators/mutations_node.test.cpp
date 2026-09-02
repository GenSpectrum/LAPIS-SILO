#include <nlohmann/json.hpp>

#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::string& nucleotide_sequence,
   const std::string& amino_acid_sequence
) {
   return {
      {"primaryKey", primary_key},
      {"segment1", {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}}},
      {"gene1", {{"sequence", amino_acid_sequence}, {"insertions", nlohmann::json::array()}}}
   };
}

const auto DATABASE_CONFIG = R"(
schema:
  instanceName: "test"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

// segment1 reference is "AT"; s1 carries a single substitution A->C at position 1, s2 matches the
// reference. gene1 is the reference "M*" for both rows and therefore contributes no (amino acid)
// mutations. So the nucleotide `mutations()` operator produces exactly one row:
//   {mutationFrom:A, mutationTo:C, sequenceName:segment1, position:1, proportion:0.5, coverage:2,
//    count:1}
const auto REFERENCE_GENOMES = ReferenceGenomes{{{"segment1", "AT"}}, {{"gene1", "M*"}}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("s1", "CT", "M*"),
         createData("s2", "AT", "M*")
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

const QueryTestScenario MUTATIONS_ALL_FIELDS = {
   .name = "MUTATIONS_ALL_FIELDS",
   .query = "default.mutations(minProportion:=0.0)",
   .expected_query_result = nlohmann::json::parse(R"([
      {"mutationFrom":"A","mutationTo":"C","sequenceName":"segment1","position":1,
       "proportion":0.5,"coverage":2,"count":1}
   ])"),
};

const QueryTestScenario MUTATIONS_MIN_PROPORTION_EXCLUDES_BELOW_THRESHOLD = {
   .name = "MUTATIONS_MIN_PROPORTION_EXCLUDES_BELOW_THRESHOLD",
   .query = "default.mutations(minProportion:=0.6)",
   .expected_query_result = nlohmann::json::array(),
};

const QueryTestScenario MUTATIONS_FIELDS_NARROWED = {
   .name = "MUTATIONS_FIELDS_NARROWED",
   .query = "default.mutations(minProportion:=0.0, fields:={position, count})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"position":1,"count":1}
   ])"),
};

const QueryTestScenario MUTATIONS_SEQUENCE_NAMES_SELECTS = {
   .name = "MUTATIONS_SEQUENCE_NAMES_SELECTS",
   .query = "default.mutations(minProportion:=0.0, sequenceNames:={segment1})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"mutationFrom":"A","mutationTo":"C","sequenceName":"segment1","position":1,
       "proportion":0.5,"coverage":2,"count":1}
   ])"),
};

const QueryTestScenario MUTATIONS_INVALID_MIN_PROPORTION = {
   .name = "MUTATIONS_INVALID_MIN_PROPORTION",
   .query = "default.mutations(minProportion:=1.5)",
   .expected_error_message = "Invalid proportion: minProportion must be in interval [0.0, 1.0]"
};

const QueryTestScenario MUTATIONS_MISSING_MIN_PROPORTION = {
   .name = "MUTATIONS_MISSING_MIN_PROPORTION",
   .query = "default.mutations()",
   .expected_error_message = "mutations() requires argument 'minProportion'"
};

const QueryTestScenario MUTATIONS_ON_NON_SCAN = {
   .name = "MUTATIONS_ON_NON_SCAN",
   .query = "default.project({primaryKey}).mutations(minProportion:=0.1)",
   .expected_error_message = "mutations() must be applied to a table scan"
};

const QueryTestScenario MUTATIONS_UNKNOWN_SEQUENCE_NAME = {
   .name = "MUTATIONS_UNKNOWN_SEQUENCE_NAME",
   .query = "default.mutations(minProportion:=0.1, sequenceNames:={unknownSegment})",
   .expected_error_message =
      "The database does not contain the Nucleotide sequence 'unknownSegment'"
};

// gene1 exists but is an amino acid sequence
const QueryTestScenario MUTATIONS_WRONG_TYPE_SEQUENCE_NAME = {
   .name = "MUTATIONS_WRONG_TYPE_SEQUENCE_NAME",
   .query = "default.mutations(minProportion:=0.1, sequenceNames:={gene1})",
   .expected_error_message = "The database does not contain the Nucleotide sequence 'gene1'"
};

const QueryTestScenario MUTATIONS_INVALID_FIELD = {
   .name = "MUTATIONS_INVALID_FIELD",
   .query = "default.mutations(minProportion:=0.1, fields:={notAField})",
   .expected_error_message =
      "The attribute 'fields' contains an invalid field 'notAField'. Valid fields are "
      "mutationFrom, mutationTo, position, sequenceName, proportion, coverage, count."
};

}  // namespace

QUERY_TEST(
   MutationsNode,
   TEST_DATA,
   ::testing::Values(
      MUTATIONS_ALL_FIELDS,
      MUTATIONS_MIN_PROPORTION_EXCLUDES_BELOW_THRESHOLD,
      MUTATIONS_FIELDS_NARROWED,
      MUTATIONS_SEQUENCE_NAMES_SELECTS,
      MUTATIONS_INVALID_MIN_PROPORTION,
      MUTATIONS_MISSING_MIN_PROPORTION,
      MUTATIONS_ON_NON_SCAN,
      MUTATIONS_UNKNOWN_SEQUENCE_NAME,
      MUTATIONS_WRONG_TYPE_SEQUENCE_NAME,
      MUTATIONS_INVALID_FIELD
   )
);
