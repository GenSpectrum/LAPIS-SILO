#include "silo/initialize/initializer.h"

#include <gtest/gtest.h>

#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

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
      database_config,
      reference_genomes,
      lineage_tree,
      phylo_tree_file,
      /*without_unaligned_columns=*/false
   );
   const auto& table_schema = schema.getDefaultTableSchema();

   ASSERT_EQ(schema.tables.size(), 1);
   const size_t expected_number_of_columns = database_config.schema.metadata.size() +
                                             reference_genomes.aa_sequence_names.size() +
                                             reference_genomes.nucleotide_sequence_names.size() * 2;
   ASSERT_EQ(table_schema.getColumnIdentifiers().size(), expected_number_of_columns);
   ASSERT_EQ(table_schema.primary_key.name, database_config.schema.primary_key);

   using silo::schema::ColumnType;
   ASSERT_TRUE(table_schema.getColumn("M").has_value());
   ASSERT_EQ(table_schema.getColumn("M").value().type, ColumnType::AMINO_ACID_SEQUENCE);
   ASSERT_TRUE(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::AminoAcid>>("M")
         .has_value()
   );
   ASSERT_EQ(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::AminoAcid>>("M")
         .value()
         ->reference_sequence,
      reference_genomes.stringToVector<silo::AminoAcid>("MADS*")
   );

   ASSERT_TRUE(table_schema.getColumn("age").has_value());
   ASSERT_EQ(table_schema.getColumn("age").value().type, ColumnType::INT32);

   ASSERT_TRUE(table_schema.getColumn("country").has_value());
   ASSERT_EQ(table_schema.getColumn("country").value().type, ColumnType::INDEXED_STRING);
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::IndexedStringColumnPartition>("country")
                  .has_value());

   ASSERT_TRUE(table_schema.getColumn("date").has_value());
   ASSERT_EQ(table_schema.getColumn("date").value().type, ColumnType::DATE);

   ASSERT_TRUE(table_schema.getColumn("division").has_value());
   ASSERT_EQ(table_schema.getColumn("division").value().type, ColumnType::INDEXED_STRING);
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::IndexedStringColumnPartition>("division"
                  )
                  .has_value());

   ASSERT_TRUE(table_schema.getColumn("main").has_value());
   ASSERT_EQ(table_schema.getColumn("main").value().type, ColumnType::NUCLEOTIDE_SEQUENCE);
   ASSERT_TRUE(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::Nucleotide>>("main"
         )
         .has_value()
   );
   ASSERT_EQ(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::Nucleotide>>("main"
         )
         .value()
         ->reference_sequence,
      reference_genomes.stringToVector<silo::Nucleotide>("ACGTACGT")
   );

   ASSERT_TRUE(table_schema.getColumn("pango_lineage").has_value());
   ASSERT_EQ(table_schema.getColumn("pango_lineage").value().type, ColumnType::INDEXED_STRING);
   ASSERT_TRUE(
      table_schema
         .getColumnMetadata<silo::storage::column::IndexedStringColumnPartition>("pango_lineage")
         .has_value()
   );
   auto pango_metadata =
      table_schema
         .getColumnMetadata<silo::storage::column::IndexedStringColumnPartition>("pango_lineage")
         .value();
   ASSERT_EQ(pango_metadata->dictionary.getValue(0), "A");
   ASSERT_EQ(pango_metadata->dictionary.getValue(1), "A.1");
   ASSERT_EQ(pango_metadata->dictionary.getValue(2), "A.11");
   ASSERT_EQ(pango_metadata->dictionary.getValue(3), "X");
   ASSERT_EQ(pango_metadata->dictionary.getValue(4), "X2");
   ASSERT_EQ(pango_metadata->dictionary.getValue(5), "Y3");

   ASSERT_TRUE(table_schema.getColumn("primaryKey").has_value());
   ASSERT_EQ(table_schema.getColumn("primaryKey").value().type, ColumnType::STRING);
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::StringColumnPartition>("primaryKey")
                  .has_value());

   ASSERT_TRUE(table_schema.getColumn("qc_value").has_value());
   ASSERT_EQ(table_schema.getColumn("qc_value").value().type, ColumnType::FLOAT);

   ASSERT_TRUE(table_schema.getColumn("region").has_value());
   ASSERT_EQ(table_schema.getColumn("region").value().type, ColumnType::INDEXED_STRING);
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::IndexedStringColumnPartition>("region")
                  .has_value());

   ASSERT_TRUE(table_schema.getColumn("testSecondSequence").has_value());
   ASSERT_EQ(
      table_schema.getColumn("testSecondSequence").value().type, ColumnType::NUCLEOTIDE_SEQUENCE
   );
   ASSERT_TRUE(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::Nucleotide>>(
            "testSecondSequence"
         )
         .has_value()
   );
   ASSERT_EQ(
      table_schema
         .getColumnMetadata<silo::storage::column::SequenceColumnPartition<silo::Nucleotide>>(
            "testSecondSequence"
         )
         .value()
         ->reference_sequence,
      reference_genomes.stringToVector<silo::Nucleotide>("ACGT")
   );

   ASSERT_TRUE(table_schema.getColumn("test_boolean_column").has_value());
   ASSERT_EQ(table_schema.getColumn("test_boolean_column").value().type, ColumnType::BOOL);

   ASSERT_TRUE(table_schema.getColumn("unaligned_main").has_value());
   ASSERT_EQ(
      table_schema.getColumn("unaligned_main").value().type, ColumnType::ZSTD_COMPRESSED_STRING
   );
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::ZstdCompressedStringColumnPartition>(
                     "unaligned_main"
                  )
                  .has_value());
   ASSERT_EQ(
      table_schema
         .getColumnMetadata<silo::storage::column::ZstdCompressedStringColumnPartition>(
            "unaligned_main"
         )
         .value()
         ->dictionary_string,
      "ACGTACGT"
   );

   ASSERT_TRUE(table_schema.getColumn("unaligned_testSecondSequence").has_value());
   ASSERT_EQ(
      table_schema.getColumn("unaligned_testSecondSequence").value().type,
      ColumnType::ZSTD_COMPRESSED_STRING
   );
   ASSERT_TRUE(table_schema
                  .getColumnMetadata<silo::storage::column::ZstdCompressedStringColumnPartition>(
                     "unaligned_testSecondSequence"
                  )
                  .has_value());
   ASSERT_EQ(
      table_schema
         .getColumnMetadata<silo::storage::column::ZstdCompressedStringColumnPartition>(
            "unaligned_testSecondSequence"
         )
         .value()
         ->dictionary_string,
      "ACGT"
   );

   ASSERT_TRUE(table_schema.getColumn("unsorted_date").has_value());
   ASSERT_EQ(table_schema.getColumn("unsorted_date").value().type, ColumnType::DATE);

   ASSERT_EQ(table_schema.primary_key.name, "primaryKey");
   ASSERT_EQ(table_schema.primary_key.type, ColumnType::STRING);
}
