#include "silo/preprocessing/sequence_info.h"

#include <gtest/gtest.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"

using silo::ReferenceGenomes;
using silo::preprocessing::SequenceInfo;

TEST(SequenceInfo, validatesSuccessfulOnCorrectFile) {
   const auto reference_genomes =
      ReferenceGenomes::readFromFile("testBaseData/exampleDataset2/reference_genomes.json");
   const SequenceInfo sequence_info(reference_genomes);

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_NO_THROW(
      sequence_info.validate(connection, "testBaseData/exampleDataset2/sample.ndjson.zst")
   );
}

TEST(SequenceInfo, failWhenTooManyGenomesInReferences) {
   const auto reference_genomes =
      ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference_genomes.json");
   const SequenceInfo sequence_info(reference_genomes);

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      sequence_info.validate(connection, "testBaseData/exampleDataset2/sample.ndjson.zst"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(SequenceInfo, failWhenTooManyGenomesInJson) {
   const auto reference_genomes =
      ReferenceGenomes::readFromFile("testBaseData/exampleDataset2/reference_genomes.json");
   const SequenceInfo sequence_info(reference_genomes);

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      sequence_info.validate(connection, "testBaseData/ndjsonFiles/oneline_second_nuc.json.zst"),
      silo::preprocessing::PreprocessingException
   );
}

TEST(SequenceInfo, failWhenTooFewAASequencesInJson) {
   const auto reference_genomes =
      ReferenceGenomes::readFromFile("testBaseData/exampleDataset2/reference_genomes.json");
   const SequenceInfo sequence_info(reference_genomes);

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      sequence_info.validate(connection, "testBaseData/ndjsonFiles/oneline_without_ORF.json.zst"),
      silo::preprocessing::PreprocessingException
   );
}