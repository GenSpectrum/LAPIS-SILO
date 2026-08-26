// End-to-end tests for BAM ingestion: a synthetic BAM is decompressed, parsed,
// projected onto reference coordinates, rendered as NDJSON keyed by the table's
// columns, and appended through the standard finalize path -- after which the
// reconstructed sequences are queryable with nucleotideEquals.
#include "rhydb/append/bam/bam_ndjson_input_stream.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>
#include <zlib.h>
#include <nlohmann/json.hpp>

#include "rhydb/database.h"
#include "rhydb/schema/database_schema.h"

using rhydb::Database;
using rhydb::append::bam::BamNdjsonInputStream;
using rhydb::schema::TableName;

namespace {

class BamBuilder {
  public:
   void u8(uint8_t value) { bytes.push_back(value); }
   void u16(uint16_t value) {
      bytes.push_back(static_cast<unsigned char>(value & 0xff));
      bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
   }
   void u32(uint32_t value) {
      for (int shift = 0; shift < 32; shift += 8) {
         bytes.push_back(static_cast<unsigned char>((value >> shift) & 0xff));
      }
   }
   void i32(int32_t value) { u32(static_cast<uint32_t>(value)); }
   void raw(std::string_view data) {
      for (char character : data) {
         bytes.push_back(static_cast<unsigned char>(character));
      }
   }
   [[nodiscard]] std::string str() const {
      return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
   }

  private:
   std::vector<unsigned char> bytes;
};

std::string gzipCompress(std::string_view data) {
   z_stream stream{};
   deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
   std::string out;
   out.resize(deflateBound(&stream, data.size()));
   stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
   stream.avail_in = static_cast<uInt>(data.size());
   stream.next_out = reinterpret_cast<Bytef*>(out.data());
   stream.avail_out = static_cast<uInt>(out.size());
   deflate(&stream, Z_FINISH);
   out.resize(out.size() - stream.avail_out);
   deflateEnd(&stream);
   return out;
}

void appendRecord(BamBuilder& bam, const BamBuilder& body) {
   const std::string body_str = body.str();
   bam.u32(static_cast<uint32_t>(body_str.size()));
   bam.raw(body_str);
}

/// Reference "ref"/len 10, three reads:
///   r1 @pos 0  4M       SEQ=ACGT   -> aligned "ACGT" @offset 0
///   r2 @pos 2  2M1I2M   SEQ=GGATT  -> aligned "GGTT" @offset 2, insertion "4:A"
///   r3 unmapped (flag 0x4)                                 -> skipped
std::string buildSampleBam() {
   BamBuilder bam;
   bam.raw("BAM\1");
   bam.u32(0);  // l_text
   bam.u32(1);  // n_ref
   bam.u32(4);  // l_name "ref\0"
   bam.raw(std::string_view{"ref\0", 4});
   bam.i32(10);  // l_ref

   {
      BamBuilder body;
      body.i32(0);   // refID
      body.i32(0);   // pos
      body.u8(3);    // l_read_name "r1\0"
      body.u8(20);   // mapq
      body.u16(0);   // bin
      body.u16(1);   // n_cigar_op
      body.u16(0);   // flag
      body.i32(4);   // l_seq
      body.i32(-1);  // next_refID
      body.i32(-1);  // next_pos
      body.i32(0);   // tlen
      body.raw(std::string_view{"r1\0", 3});
      body.u32((4U << 4) | 0U);  // 4M
      body.u8(0x12);             // A,C
      body.u8(0x48);             // G,T
      body.u8(20);
      body.u8(20);
      body.u8(20);
      body.u8(20);
      appendRecord(bam, body);
   }

   {
      BamBuilder body;
      body.i32(0);   // refID
      body.i32(2);   // pos
      body.u8(3);    // l_read_name "r2\0"
      body.u8(20);   // mapq
      body.u16(0);   // bin
      body.u16(3);   // n_cigar_op
      body.u16(0);   // flag
      body.i32(5);   // l_seq
      body.i32(-1);  // next_refID
      body.i32(-1);  // next_pos
      body.i32(0);   // tlen
      body.raw(std::string_view{"r2\0", 3});
      body.u32((2U << 4) | 0U);  // 2M
      body.u32((1U << 4) | 1U);  // 1I
      body.u32((2U << 4) | 0U);  // 2M
      body.u8(0x44);             // G,G
      body.u8(0x18);             // A,T
      body.u8(0x80);             // T,pad
      body.u8(0xFF);             // QUAL unavailable
      body.u8(0xFF);
      body.u8(0xFF);
      body.u8(0xFF);
      body.u8(0xFF);
      appendRecord(bam, body);
   }

   {
      BamBuilder body;
      body.i32(-1);  // refID (unmapped)
      body.i32(-1);  // pos
      body.u8(3);    // l_read_name "r3\0"
      body.u8(0);    // mapq
      body.u16(0);   // bin
      body.u16(0);   // n_cigar_op
      body.u16(4);   // flag: unmapped
      body.i32(0);   // l_seq
      body.i32(-1);  // next_refID
      body.i32(-1);  // next_pos
      body.i32(0);   // tlen
      body.raw(std::string_view{"r3\0", 3});
      appendRecord(bam, body);
   }

   return gzipCompress(bam.str());
}

Database buildReadsDatabase() {
   Database database;
   database.createNucleotideSequenceTable("reads", "qname", "seq", "AAAAAAAAAA", {"cigar"});
   return database;
}

}  // namespace

TEST(BamIngest, rendersSchemaDrivenNdjsonPerKeptRead) {
   Database database = buildReadsDatabase();
   const auto& schema = *database.tables.at(TableName{"reads"})->schema;

   std::stringstream bam{buildSampleBam()};
   BamNdjsonInputStream ndjson{bam, schema};

   std::vector<nlohmann::json> lines;
   std::string line;
   while (std::getline(ndjson, line)) {
      lines.push_back(nlohmann::json::parse(line));
   }

   ASSERT_EQ(lines.size(), 2U);  // the unmapped r3 is skipped

   EXPECT_EQ(lines[0]["qname"], "r1");
   EXPECT_EQ(lines[0]["cigar"], "4M");
   EXPECT_EQ(lines[0]["seq"]["sequence"], "ACGT");
   EXPECT_EQ(lines[0]["seq"]["offset"], 0);
   EXPECT_TRUE(lines[0]["seq"]["insertions"].empty());

   EXPECT_EQ(lines[1]["qname"], "r2");
   EXPECT_EQ(lines[1]["cigar"], "2M1I2M");
   EXPECT_EQ(lines[1]["seq"]["sequence"], "GGTT");
   EXPECT_EQ(lines[1]["seq"]["offset"], 2);
   ASSERT_EQ(lines[1]["seq"]["insertions"].size(), 1U);
   EXPECT_EQ(lines[1]["seq"]["insertions"][0], "4:A");
}

TEST(BamIngest, appendsReadsAndMakesSequencesQueryable) {
   Database database = buildReadsDatabase();

   std::stringstream bam{buildSampleBam()};
   database.appendBamData(TableName{"reads"}, bam);

   // Reference is all 'A'. Position is 1-based in nucleotideEquals.
   // Genome index 1 ('C') is covered only by r1 (row 0); r2 covers [2,6).
   const auto only_r1 = database.getFilteredBitmap(
      "reads", "nucleotideEquals(position:=2, symbol:='C', sequenceName:='seq')"
   );
   EXPECT_EQ(only_r1.cardinality(), 1U);
   EXPECT_TRUE(only_r1.contains(0));

   // Genome index 2 ('G') is 'G' in both r1 ("ACGT") and r2 ("GGTT").
   const auto both = database.getFilteredBitmap(
      "reads", "nucleotideEquals(position:=3, symbol:='G', sequenceName:='seq')"
   );
   EXPECT_EQ(both.cardinality(), 2U);

   // No read has 'C' at genome index 0 (r1 has 'A' there; r2 does not cover it).
   const auto none = database.getFilteredBitmap(
      "reads", "nucleotideEquals(position:=1, symbol:='C', sequenceName:='seq')"
   );
   EXPECT_EQ(none.cardinality(), 0U);
}
