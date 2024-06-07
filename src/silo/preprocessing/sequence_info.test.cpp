#include "silo/preprocessing/sequence_info.h"

#include <gtest/gtest.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"

using silo::ReferenceGenomes;
using silo::preprocessing::SequenceInfo;

TEST(SequenceInfo, validatesSuccessfulOnCorrectFile) {
   const auto reference_genomes = ReferenceGenomes::readFromFile(
      "testBaseData/exampleDataset1000Sequences/reference_genomes.json"
   );

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_NO_THROW(SequenceInfo::validateNdjsonFile(
      reference_genomes, connection, "testBaseData/exampleDataset1000Sequences/sample.ndjson.zst"
   ));
}

TEST(SequenceInfo, failWhenTooManyGenomesInReferences) {
   const auto reference_genomes =
      ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference_genomes.json");

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      SequenceInfo::validateNdjsonFile(
         reference_genomes, connection, "testBaseData/exampleDataset1000Sequences/sample.ndjson.zst"
      ),
      silo::preprocessing::PreprocessingException
   );
}

TEST(SequenceInfo, failWhenTooManyGenomesInJson) {
   const auto reference_genomes = ReferenceGenomes::readFromFile(
      "testBaseData/exampleDataset1000Sequences/reference_genomes.json"
   );

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      SequenceInfo::validateNdjsonFile(
         reference_genomes, connection, "testBaseData/ndjsonFiles/oneline_second_nuc.json.zst"
      ),
      silo::preprocessing::PreprocessingException
   );
}

TEST(SequenceInfo, failWhenTooFewAASequencesInJson) {
   const auto reference_genomes = ReferenceGenomes::readFromFile(
      "testBaseData/exampleDataset1000Sequences/reference_genomes.json"
   );

   duckdb::DuckDB duckdb;
   duckdb::Connection connection(duckdb);
   ASSERT_THROW(
      SequenceInfo::validateNdjsonFile(
         reference_genomes, connection, "testBaseData/ndjsonFiles/oneline_without_ORF.json.zst"
      ),
      silo::preprocessing::PreprocessingException
   );
}