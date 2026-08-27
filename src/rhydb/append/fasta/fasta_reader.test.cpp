#include "rhydb/append/fasta/fasta_reader.h"

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "rhydb/append/fasta/fasta_exception.h"

using rhydb::append::fasta::FastaException;
using rhydb::append::fasta::FastaReader;
using rhydb::append::fasta::FastaRecord;

namespace {

std::vector<FastaRecord> readAll(const std::string& text) {
   std::stringstream stream{text};
   FastaReader reader{stream};
   std::vector<FastaRecord> records;
   while (auto record = reader.next()) {
      records.push_back(std::move(*record));
   }
   return records;
}

}  // namespace

TEST(FastaReader, readsSingleRecord) {
   auto records = readAll(">seq1\nACGT\n");
   ASSERT_EQ(records.size(), 1U);
   EXPECT_EQ(records[0].identifier, "seq1");
   EXPECT_EQ(records[0].sequence, "ACGT");
}

TEST(FastaReader, concatenatesWrappedSequenceLines) {
   auto records = readAll(">seq1\nACGT\nACGT\nAC\n");
   ASSERT_EQ(records.size(), 1U);
   EXPECT_EQ(records[0].sequence, "ACGTACGTAC");
}

TEST(FastaReader, readsMultipleRecords) {
   auto records = readAll(">a\nAAAA\n>b\nCCCC\n>c\nGGGG\n");
   ASSERT_EQ(records.size(), 3U);
   EXPECT_EQ(records[0].identifier, "a");
   EXPECT_EQ(records[1].identifier, "b");
   EXPECT_EQ(records[2].identifier, "c");
   EXPECT_EQ(records[2].sequence, "GGGG");
}

TEST(FastaReader, identifierIsFirstTokenAndDescriptionIsIgnored) {
   auto records = readAll(">seq1 some description here\nACGT\n");
   ASSERT_EQ(records.size(), 1U);
   EXPECT_EQ(records[0].identifier, "seq1");
}

TEST(FastaReader, handlesCrlfAndBlankLines) {
   auto records = readAll(">seq1\r\nAC\r\n\r\nGT\r\n");
   ASSERT_EQ(records.size(), 1U);
   EXPECT_EQ(records[0].identifier, "seq1");
   EXPECT_EQ(records[0].sequence, "ACGT");
}

TEST(FastaReader, handlesMissingTrailingNewline) {
   auto records = readAll(">seq1\nACGT");
   ASSERT_EQ(records.size(), 1U);
   EXPECT_EQ(records[0].sequence, "ACGT");
}

TEST(FastaReader, emptyInputYieldsNoRecords) {
   EXPECT_TRUE(readAll("").empty());
   EXPECT_TRUE(readAll("\n\n\n").empty());
}

TEST(FastaReader, throwsOnSequenceBeforeHeader) {
   std::stringstream stream{"ACGT\n>seq1\nACGT\n"};
   FastaReader reader{stream};
   EXPECT_THROW(reader.next(), FastaException);
}

TEST(FastaReader, throwsOnHeaderWithoutIdentifier) {
   std::stringstream stream{">\nACGT\n"};
   FastaReader reader{stream};
   EXPECT_THROW(reader.next(), FastaException);
}
