// End-to-end tests for FASTA ingestion: records are rendered as NDJSON keyed by
// the table columns (identifier -> primary key, sequence -> the sequence column)
// and appended through the standard finalize path, after which the sequences are
// queryable with nucleotideEquals.
#include "rhydb/append/fasta/fasta_ndjson_input_stream.h"

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "rhydb/database.h"

using rhydb::Database;
using rhydb::append::fasta::FastaNdjsonInputStream;
using rhydb::schema::TableName;

namespace {

constexpr const char* SAMPLE_FASTA = ">s1\nACGT\n>s2\nAAAA\n";

Database buildSeqDatabase() {
   Database database;
   // Schema: id (STRING primary key) + seq (NUCLEOTIDE_SEQUENCE, reference "AAAA").
   database.createNucleotideSequenceTable("seqs", "id", "seq", "AAAA");
   return database;
}

}  // namespace

TEST(FastaIngest, rendersSchemaDrivenNdjsonPerRecord) {
   Database database = buildSeqDatabase();
   const auto& schema = *database.tables.at(TableName{"seqs"})->schema;

   std::stringstream fasta{SAMPLE_FASTA};
   FastaNdjsonInputStream ndjson{fasta, schema};

   std::vector<nlohmann::json> lines;
   std::string line;
   while (std::getline(ndjson, line)) {
      lines.push_back(nlohmann::json::parse(line));
   }

   ASSERT_EQ(lines.size(), 2U);
   EXPECT_EQ(lines[0]["id"], "s1");
   EXPECT_EQ(lines[0]["seq"]["sequence"], "ACGT");
   EXPECT_EQ(lines[0]["seq"]["offset"], 0);
   EXPECT_TRUE(lines[0]["seq"]["insertions"].empty());
   EXPECT_EQ(lines[1]["id"], "s2");
   EXPECT_EQ(lines[1]["seq"]["sequence"], "AAAA");
}

TEST(FastaIngest, appendsRecordsAndMakesSequencesQueryable) {
   Database database = buildSeqDatabase();

   std::stringstream fasta{SAMPLE_FASTA};
   database.appendFastaData(TableName{"seqs"}, fasta);

   // Reference is "AAAA"; position is 1-based in nucleotideEquals.
   // s1 = ACGT (row 0), s2 = AAAA (row 1).
   const auto has_c_at_2 = database.getFilteredBitmap(
      "seqs", "nucleotideEquals(position:=2, symbol:='C', sequenceName:='seq')"
   );
   EXPECT_EQ(has_c_at_2.cardinality(), 1U);
   EXPECT_TRUE(has_c_at_2.contains(0));

   const auto has_a_at_1 = database.getFilteredBitmap(
      "seqs", "nucleotideEquals(position:=1, symbol:='A', sequenceName:='seq')"
   );
   EXPECT_EQ(has_a_at_1.cardinality(), 2U);

   const auto has_a_at_2 = database.getFilteredBitmap(
      "seqs", "nucleotideEquals(position:=2, symbol:='A', sequenceName:='seq')"
   );
   EXPECT_EQ(has_a_at_2.cardinality(), 1U);
   EXPECT_TRUE(has_a_at_2.contains(1));
}

TEST(FastaIngest, rejectsSchemaWithUnsupportedColumn) {
   Database database;
   // An extra string column has no FASTA data to fill it -> ingest must reject it.
   database.createNucleotideSequenceTable("seqs", "id", "seq", "AAAA", {"country"});
   std::stringstream fasta{SAMPLE_FASTA};
   EXPECT_ANY_THROW(database.appendFastaData(TableName{"seqs"}, fasta));
}
