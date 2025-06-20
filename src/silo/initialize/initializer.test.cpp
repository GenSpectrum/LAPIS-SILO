#include "silo/initialize/initializer.h"

#include <gtest/gtest.h>

TEST(Initializer, correctlyCreatesSchemaFromInitializationFiles) {
   silo::config::DatabaseConfig database_config =
      silo::config::DatabaseConfig::getValidatedConfigFromFile(
         "testBaseData/unitTestDummyDataset/database_config.yaml"
      );
   silo::ReferenceGenomes reference_genomes = silo::ReferenceGenomes::readFromFile(
      "testBaseData/unitTestDummyDataset/reference_genomes.json"
   );
   silo::common::PhyloTree phylo_tree_file = silo::common::PhyloTree::fromNewickFile(
      "testBaseData/unitTestDummyDataset/phylogenetic_tree.nwk"
   );
   auto lineage_tree = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFile(
      silo::preprocessing::LineageDefinitionFile::fromYAMLString(R"(
A:
  aliases:
  - X
A.1:
  parents:
  - A
A.11:
  aliases:
  - X2
  - Y3
  parents:
  - A
)")
   );
   auto schema = silo::initialize::Initializer::createSchemaFromConfigFiles(
      database_config, reference_genomes, lineage_tree, phylo_tree_file
   );

   ASSERT_EQ(schema.tables.size(), 1);
   const size_t expected_number_of_columns = database_config.schema.metadata.size() +
                                             reference_genomes.aa_sequence_names.size() +
                                             reference_genomes.nucleotide_sequence_names.size() * 2;
   ASSERT_EQ(
      schema.getDefaultTableSchema().getColumnIdentifiers().size(), expected_number_of_columns
   );
   ASSERT_EQ(schema.getDefaultTableSchema().primary_key.name, database_config.schema.primary_key);

   const auto actual_database_schema_dump = YAML::Dump(schema.toYAML());
   SPDLOG_INFO("\n{}", actual_database_schema_dump);
   ASSERT_EQ(actual_database_schema_dump, R"(default:
  columns:
    - name: E
      type: aminoAcidSequence
      metadata:
        referenceSequence: MYSF*
    - name: M
      type: aminoAcidSequence
      metadata:
        referenceSequence: MADS*
    - name: age
      type: integer
    - name: country
      type: indexedString
      metadata:
        dictionary:
          []
    - name: date
      type: date
    - name: division
      type: indexedString
      metadata:
        dictionary:
          []
    - name: main
      type: nucleotideSequence
      metadata:
        referenceSequence: ACGTACGT
    - name: pango_lineage
      type: indexedString
      metadata:
        dictionary:
          - A
          - A.1
          - A.11
          - X
          - X2
          - Y3
        lineageTree:
          A:
            aliases:
              - X
          A.1:
            parents:
              - A
          A.11:
            aliases:
              - X2
              - Y3
            parents:
              - A
    - name: primaryKey
      type: string
      metadata:
        sharedSuffixTable:
          []
    - name: qc_value
      type: float
    - name: region
      type: indexedString
      metadata:
        dictionary:
          []
    - name: testSecondSequence
      type: nucleotideSequence
      metadata:
        referenceSequence: ACGT
    - name: test_boolean_column
      type: bool
    - name: unaligned_main
      type: zstdCompressedString
      metadata:
        zstdDictionary: ACGTACGT
    - name: unaligned_testSecondSequence
      type: zstdCompressedString
      metadata:
        zstdDictionary: ACGT
    - name: unsorted_date
      type: date
  primaryKey: primaryKey)");

   auto database_schema_from_dump =
      silo::schema::DatabaseSchema::fromYAML(YAML::Load(actual_database_schema_dump));
   auto round_trip_database_schema_dump = YAML::Dump(database_schema_from_dump.toYAML());
   ASSERT_EQ(round_trip_database_schema_dump, actual_database_schema_dump);
}
