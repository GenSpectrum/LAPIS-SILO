#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::string& nucleotide_sequence,
   const std::string& amino_acid_sequence
) {
   return {
      {"primaryKey", primary_key},
      {"segment1", {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}}},
      {"gene1", {{"sequence", amino_acid_sequence}, {"insertions", nlohmann::json::array()}}},
      {"gene2", nullptr}
   };
}

// segment1 reference: ATGCN (length 5)
// gene1 reference:    M*    (length 2; * = STOP codon, a definitive AA symbol)
//
// Sequences (segment1 | gene1):
//   seq_ref:       ATGCN | M*   (0 nuc diffs; 0 AA diffs from their respective references)
//   seq_1mut:      CTGCN | C*   (1 nuc diff: pos1 A→C;  1 AA diff: pos1 M→C)
//   seq_2mut:      CTCCN | M*   (2 nuc diffs: pos1, pos3; 0 AA diffs)
//   seq_3mut:      CTCTN | M*   (3 nuc diffs: pos1, pos3, pos4; 0 AA diffs)
//   seq_all_n:     NNNNN | M*   (0 conservative nuc diffs; 0 AA diffs)
//   seq_mixed_amb: RTGCN | M*   (0 conservative nuc diffs; 0 AA diffs)

const auto DATABASE_CONFIG = R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "test"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES =
   ReferenceGenomes{{{"segment1", "ATGCN"}}, {{"gene1", "M*"}, {"gene2", "M*"}}};

// Note: reference has N at position 5, so that position is always skipped in profile comparisons.
// Effective profile length for distance counting = 4 positions (ATGC).

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("seq_ref", "ATGCN", "M*"),       // 0 diffs from reference profile
         createData("seq_1mut", "CTGCN", "C*"),      // 1 diff from reference profile (pos1)
         createData("seq_2mut", "CTCCN", "M*"),      // 2 diffs from reference profile (pos1, pos3)
         createData("seq_3mut", "CTCTN", "M*"),      // 3 diffs from reference profile (pos1,3,4)
         createData("seq_all_n", "NNNNN", "M*"),     // 0 conservative diffs (N is compatible)
         createData("seq_mixed_amb", "RTGCN", "M*")  // 0 diffs: R∈AMBIGUITY[A], conservative
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

// ------ Tests using mutations input method ------

// Profile = reference (empty mutations list), distance=0
// Should match: seq_ref (0 diffs), seq_all_n (0 diffs), seq_mixed_amb (0 diffs, R compatible w/ A)
const QueryTestScenario MUTATIONS_DISTANCE_0 = {
   .name = "MUTATIONS_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "mutations": []
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// Profile = reference, distance=1 → matches 0 or 1 difference
const QueryTestScenario MUTATIONS_DISTANCE_1 = {
   .name = "MUTATIONS_DISTANCE_1",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 1,
       "mutations": []
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_1mut"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// Profile = reference, distance=2 → matches 0, 1, or 2 differences
const QueryTestScenario MUTATIONS_DISTANCE_2 = {
   .name = "MUTATIONS_DISTANCE_2",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 2,
       "mutations": []
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_1mut"},{"primaryKey":"seq_2mut"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// Profile with explicit mutation C at position 1 (0-indexed: 0), distance=0
// Profile = CTGCN → should match seq_1mut (which has CTGCN) and seq_all_n
// seq_mixed_amb does not match, because C is not in {A,G}=R
const QueryTestScenario MUTATIONS_WITH_PROFILE_DISTANCE_0 = {
   .name = "MUTATIONS_WITH_PROFILE_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "mutations": [{"position": 1, "symbol": "C"}]
     }
   })"),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"seq_1mut"},{"primaryKey":"seq_all_n"}])")
};

// ------ Tests using querySequence input method ------

// querySequence = "ATGCN" (same as reference), distance=0
// Same as MUTATIONS_DISTANCE_0 but N at position 5 is skipped (profile has N = SYMBOL_MISSING)
const QueryTestScenario QUERY_SEQUENCE_DISTANCE_0 = {
   .name = "QUERY_SEQUENCE_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "querySequence": "ATGCN"
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// querySequence wrong length
const QueryTestScenario QUERY_SEQUENCE_WRONG_LENGTH = {
   .name = "QUERY_SEQUENCE_WRONG_LENGTH",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "querySequence": "ATG"
     }
   })"),
   .expected_error_message =
      "querySequence length 3 does not match the reference sequence length 5 for Nucleotide "
      "MutationProfile"
};

// ------ Tests using sequenceId input method ------

// Use seq_1mut as profile (CTGCN), distance=0
// Should match: seq_1mut (exact match), seq_all_n (N compatible with everything)
const QueryTestScenario SEQUENCE_ID_DISTANCE_0 = {
   .name = "SEQUENCE_ID_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "sequenceId": "seq_1mut"
     }
   })"),
   .expected_query_result =
      nlohmann::json::parse(R"([{"primaryKey":"seq_1mut"},{"primaryKey":"seq_all_n"}])")
};

// sequenceId not found
const QueryTestScenario SEQUENCE_ID_NOT_FOUND = {
   .name = "SEQUENCE_ID_NOT_FOUND",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "sequenceId": "nonexistent_id"
     }
   })"),
   .expected_error_message =
      "No sequence found with primary key 'nonexistent_id' in Nucleotide MutationProfile"
};

// No input method provided
const QueryTestScenario NO_INPUT_METHOD = {
   .name = "NO_INPUT_METHOD",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0
     }
   })"),
   .expected_error_message =
      "Exactly one of 'querySequence', 'sequenceId', or 'mutations' must be provided in a "
      "Nucleotide MutationProfile expression, but 0 were provided"
};

// Two input methods provided
const QueryTestScenario TWO_INPUT_METHODS = {
   .name = "TWO_INPUT_METHODS",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "NucleotideMutationProfile",
       "distance": 0,
       "mutations": [],
       "querySequence": "ATGCN"
     }
   })"),
   .expected_error_message =
      "Exactly one of 'querySequence', 'sequenceId', or 'mutations' must be provided in a "
      "Nucleotide MutationProfile expression, but 2 were provided"
};

// ------ AminoAcid tests ------
// Only seq_1mut differs from the gene1 reference (M→C at pos1). All other rows have "M*".

// Profile = gene1 reference (empty mutations list), distance=0
// Matches every row whose AA sequence == "M*"; excludes seq_1mut ("C*", 1 diff).
const QueryTestScenario AA_MUTATIONS_REFERENCE_DISTANCE_0 = {
   .name = "AA_MUTATIONS_REFERENCE_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "gene1",
       "distance": 0,
       "mutations": []
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_2mut"},{"primaryKey":"seq_3mut"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// Profile = gene1 reference, distance=1
// seq_1mut has exactly 1 AA diff (M→C) which is ≤ 1 → all 6 rows match.
const QueryTestScenario AA_MUTATIONS_REFERENCE_DISTANCE_1 = {
   .name = "AA_MUTATIONS_REFERENCE_DISTANCE_1",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "gene1",
       "distance": 1,
       "mutations": []
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_1mut"},{"primaryKey":"seq_2mut"},{"primaryKey":"seq_3mut"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// Profile = "C*" via mutations=[{pos:1, sym:"C"}], distance=0
// Only seq_1mut has C at pos1; all others have M (not compatible with C) → only seq_1mut matches.
const QueryTestScenario AA_MUTATIONS_WITH_PROFILE_DISTANCE_0 = {
   .name = "AA_MUTATIONS_WITH_PROFILE_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "gene1",
       "distance": 0,
       "mutations": [{"position": 1, "symbol": "C"}]
     }
   })"),
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"seq_1mut"}])")
};

// querySequence = "M*" (same as gene1 reference), distance=0 — exercises querySequence parsing for
// AA
const QueryTestScenario AA_QUERY_SEQUENCE_DISTANCE_0 = {
   .name = "AA_QUERY_SEQUENCE_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "gene1",
       "distance": 0,
       "querySequence": "M*"
     }
   })"),
   .expected_query_result = nlohmann::json::parse(
      R"([{"primaryKey":"seq_ref"},{"primaryKey":"seq_2mut"},{"primaryKey":"seq_3mut"},{"primaryKey":"seq_all_n"},{"primaryKey":"seq_mixed_amb"}])"
   )
};

// sequenceId = "seq_1mut" → reconstructed profile is "C*", distance=0
// Only seq_1mut itself has "C*" → only seq_1mut matches.
const QueryTestScenario AA_SEQUENCE_ID_DISTANCE_0 = {
   .name = "AA_SEQUENCE_ID_DISTANCE_0",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "gene1",
       "distance": 0,
       "sequenceId": "seq_1mut"
     }
   })"),
   .expected_query_result = nlohmann::json::parse(R"([{"primaryKey":"seq_1mut"}])")
};

// sequenceName refers to a gene that does not exist in the schema → error
const QueryTestScenario AA_INVALID_SEQUENCE_NAME = {
   .name = "AA_INVALID_SEQUENCE_NAME",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "sequenceName": "nonexistent_gene",
       "distance": 0,
       "mutations": []
     }
   })"),
   .expected_error_message =
      "Database does not contain the AminoAcid Sequence with name: 'nonexistent_gene'"
};

// No sequenceName provided and no default AA sequence in the config → error
const QueryTestScenario AA_NO_SEQUENCE_NAME = {
   .name = "AA_NO_SEQUENCE_NAME",
   .query = nlohmann::json::parse(R"({
     "action": {"type": "Details", "fields": ["primaryKey"]},
     "filterExpression": {
       "type": "AminoAcidMutationProfile",
       "distance": 0,
       "mutations": []
     }
   })"),
   .expected_error_message =
      "Database does not have a default sequence name for AminoAcid sequences. "
      "You need to provide the sequence name with the AminoAcid MutationProfile filter."
};

}  // namespace

QUERY_TEST(
   MutationProfile,
   TEST_DATA,
   ::testing::Values(
      MUTATIONS_DISTANCE_0,
      MUTATIONS_DISTANCE_1,
      MUTATIONS_DISTANCE_2,
      MUTATIONS_WITH_PROFILE_DISTANCE_0,
      QUERY_SEQUENCE_DISTANCE_0,
      QUERY_SEQUENCE_WRONG_LENGTH,
      SEQUENCE_ID_DISTANCE_0,
      SEQUENCE_ID_NOT_FOUND,
      NO_INPUT_METHOD,
      TWO_INPUT_METHODS
   )
);

QUERY_TEST(
   AminoAcidMutationProfile,
   TEST_DATA,
   ::testing::Values(
      AA_MUTATIONS_REFERENCE_DISTANCE_0,
      AA_MUTATIONS_REFERENCE_DISTANCE_1,
      AA_MUTATIONS_WITH_PROFILE_DISTANCE_0,
      AA_QUERY_SEQUENCE_DISTANCE_0,
      AA_SEQUENCE_ID_DISTANCE_0,
      AA_INVALID_SEQUENCE_NAME,
      AA_NO_SEQUENCE_NAME
   )
);
