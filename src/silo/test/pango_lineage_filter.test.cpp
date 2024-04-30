#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::PangoLineageAliasLookup;
using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

static const std::string SOME_BASE_PANGO_LINEAGE = "BASE.1";
static const std::string SOME_SUBLINEAGE = "CHILD.1";

nlohmann::json createDataWithPangoLineageValue(const std::string& primaryKey, std::string value) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"pango_lineage", value}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}}
   };
}

nlohmann::json createDataWithPangoLineageNullValue(const std::string& primaryKey) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"pango_lineage", nullptr}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}}
   };
}
const std::vector<nlohmann::json> DATA = {
   createDataWithPangoLineageValue("id_0", SOME_BASE_PANGO_LINEAGE),
   createDataWithPangoLineageValue("id_1", SOME_BASE_PANGO_LINEAGE),
   createDataWithPangoLineageValue("id_2", SOME_SUBLINEAGE),
   createDataWithPangoLineageNullValue("id_3")
};

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata =
          {{.name = "primaryKey", .type = ValueType::STRING},
           {.name = "pango_lineage", .type = ValueType::PANGOLINEAGE, .generate_index = true}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const auto ALIAS_LOOKUP = PangoLineageAliasLookup{{{"CHILD", {"BASE.1.1.1"}}}};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .alias_lookup = ALIAS_LOOKUP
};

nlohmann::json createPangoLineageQuery(const nlohmann::json value, bool include_sublineages) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "PangoLineage"},
        {"column", "pango_lineage"},
        {"value", value},
        {"includeSublineages", include_sublineages}}}
   };
}

const QueryTestScenario PANGO_LINEAGE_FILTER_SCENARIO = {
   .name = "pangoLineageFilter",
   .query = createPangoLineageQuery(SOME_BASE_PANGO_LINEAGE, false),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"pango_lineage", SOME_BASE_PANGO_LINEAGE}},
       {{"primaryKey", "id_1"}, {"pango_lineage", SOME_BASE_PANGO_LINEAGE}}}
   )
};

const QueryTestScenario PANGO_LINEAGE_FILTER_INCLUDING_SUBLINEAGES_SCENARIO = {
   .name = "pangoLineageFilterIncludingSublineages",
   .query = createPangoLineageQuery(SOME_BASE_PANGO_LINEAGE, true),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"pango_lineage", SOME_BASE_PANGO_LINEAGE}},
       {{"primaryKey", "id_1"}, {"pango_lineage", SOME_BASE_PANGO_LINEAGE}},
       {{"primaryKey", "id_2"}, {"pango_lineage", SOME_SUBLINEAGE}}}
   )
};

const QueryTestScenario PANGO_LINEAGE_FILTER_NULL_SCENARIO = {
   .name = "pangoLineageFilterNull",
   .query = createPangoLineageQuery(nullptr, false),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_3"}, {"pango_lineage", nullptr}}})
};

const QueryTestScenario PANGO_LINEAGE_FILTER_NULL_INCLUDING_SUBLINEAGES_SCENARIO = {
   .name = "pangoLineageFilterNullIncludingSublineages",
   .query = createPangoLineageQuery(nullptr, true),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_3"}, {"pango_lineage", nullptr}}})
};

QUERY_TEST(
   PangoLineageFilterTest,
   TEST_DATA,
   ::testing::Values(
      PANGO_LINEAGE_FILTER_SCENARIO,
      PANGO_LINEAGE_FILTER_INCLUDING_SUBLINEAGES_SCENARIO,
      PANGO_LINEAGE_FILTER_NULL_SCENARIO,
      PANGO_LINEAGE_FILTER_NULL_INCLUDING_SUBLINEAGES_SCENARIO
   )
);
