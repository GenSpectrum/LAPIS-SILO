#include "rhydb/append/bam/bam_reader.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <zlib.h>
#include <gtest/gtest.h>

#include "rhydb/append/bam/cigar.h"

using rhydb::append::bam::BamReader;
using rhydb::append::bam::CigarOp;
using rhydb::append::bam::projectReadOntoReference;

namespace {

/// Little-endian byte-buffer builder for assembling a synthetic BAM file.
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

/// A two-record BAM over a single reference "chr1" of length 100:
///   read1 @pos 10, 4M, SEQ=ACGT, QUAL=[30,30,30,30]
///   r2    @pos 20, 2M1I2M, flag 16 (reverse), SEQ=ACGTA, QUAL unavailable
std::string buildSampleBam() {
   BamBuilder bam;
   bam.raw("BAM\1");
   bam.u32(0);  // l_text
   bam.u32(1);  // n_ref
   bam.u32(5);  // l_name for "chr1\0"
   bam.raw(std::string_view{"chr1\0", 5});
   bam.i32(100);  // l_ref

   // Record 1.
   {
      BamBuilder body;
      body.i32(0);   // refID
      body.i32(10);  // pos
      body.u8(6);    // l_read_name ("read1\0")
      body.u8(30);   // mapq
      body.u16(0);   // bin
      body.u16(1);   // n_cigar_op
      body.u16(0);   // flag
      body.i32(4);   // l_seq
      body.i32(-1);  // next_refID
      body.i32(-1);  // next_pos
      body.i32(0);   // tlen
      body.raw(std::string_view{"read1\0", 6});
      body.u32((4U << 4) | 0U);  // 4M
      body.u8(0x12);             // A,C
      body.u8(0x48);             // G,T
      body.u8(30);
      body.u8(30);
      body.u8(30);
      body.u8(30);
      const std::string body_str = body.str();
      bam.u32(static_cast<uint32_t>(body_str.size()));
      bam.raw(body_str);
   }

   // Record 2.
   {
      BamBuilder body;
      body.i32(0);    // refID
      body.i32(20);   // pos
      body.u8(3);     // l_read_name ("r2\0")
      body.u8(0);     // mapq
      body.u16(0);    // bin
      body.u16(3);    // n_cigar_op
      body.u16(16);   // flag (reverse strand)
      body.i32(5);    // l_seq
      body.i32(-1);   // next_refID
      body.i32(-1);   // next_pos
      body.i32(0);    // tlen
      body.raw(std::string_view{"r2\0", 3});
      body.u32((2U << 4) | 0U);  // 2M
      body.u32((1U << 4) | 1U);  // 1I
      body.u32((2U << 4) | 0U);  // 2M
      body.u8(0x12);             // A,C
      body.u8(0x48);             // G,T
      body.u8(0x10);             // A, pad
      body.u8(0xFF);             // QUAL unavailable
      body.u8(0xFF);
      body.u8(0xFF);
      body.u8(0xFF);
      body.u8(0xFF);
      const std::string body_str = body.str();
      bam.u32(static_cast<uint32_t>(body_str.size()));
      bam.raw(body_str);
   }

   return gzipCompress(bam.str());
}

}  // namespace

TEST(BamReader, parsesHeaderReferenceDictionary) {
   std::stringstream stream{buildSampleBam()};
   BamReader reader{stream};
   ASSERT_EQ(reader.references().size(), 1U);
   EXPECT_EQ(reader.references().at(0).name, "chr1");
   EXPECT_EQ(reader.references().at(0).length, 100);
}

TEST(BamReader, decodesFirstRecordFields) {
   std::stringstream stream{buildSampleBam()};
   BamReader reader{stream};
   auto record = reader.next();
   ASSERT_TRUE(record.has_value());
   EXPECT_EQ(record->reference_id, 0);
   EXPECT_EQ(record->position, 10);
   EXPECT_EQ(record->mapping_quality, 30);
   EXPECT_EQ(record->flag, 0);
   EXPECT_EQ(record->read_name, "read1");
   ASSERT_EQ(record->cigar.size(), 1U);
   EXPECT_EQ(record->cigar.at(0).length, 4U);
   EXPECT_EQ(record->cigar.at(0).op, CigarOp::MATCH);
   EXPECT_EQ(record->sequence, "ACGT");
   EXPECT_EQ(record->quality, "????");  // 30 + 33 == '?'
   EXPECT_EQ(record->next_reference_id, -1);
   EXPECT_EQ(record->template_length, 0);
}

TEST(BamReader, decodesSecondRecordAndCigarAndMissingQuality) {
   std::stringstream stream{buildSampleBam()};
   BamReader reader{stream};
   ASSERT_TRUE(reader.next().has_value());  // skip record 1
   auto record = reader.next();
   ASSERT_TRUE(record.has_value());
   EXPECT_EQ(record->position, 20);
   EXPECT_EQ(record->read_name, "r2");
   EXPECT_EQ(record->flag, 16);
   ASSERT_EQ(record->cigar.size(), 3U);
   EXPECT_EQ(record->cigar.at(0).op, CigarOp::MATCH);
   EXPECT_EQ(record->cigar.at(1).op, CigarOp::INSERTION);
   EXPECT_EQ(record->cigar.at(2).op, CigarOp::MATCH);
   EXPECT_EQ(record->sequence, "ACGTA");
   EXPECT_EQ(record->quality, "*");
}

TEST(BamReader, returnsNulloptAfterLastRecord) {
   std::stringstream stream{buildSampleBam()};
   BamReader reader{stream};
   ASSERT_TRUE(reader.next().has_value());
   ASSERT_TRUE(reader.next().has_value());
   EXPECT_FALSE(reader.next().has_value());
   EXPECT_FALSE(reader.next().has_value());  // idempotent at EOF
}

TEST(BamReader, parsedRecordProjectsOntoReference) {
   std::stringstream stream{buildSampleBam()};
   BamReader reader{stream};
   ASSERT_TRUE(reader.next().has_value());  // skip record 1
   auto record = reader.next();
   ASSERT_TRUE(record.has_value());
   auto aligned =
      projectReadOntoReference(record->position, record->sequence, record->cigar);
   ASSERT_TRUE(aligned.has_value());
   EXPECT_EQ(aligned->offset, 20U);
   EXPECT_EQ(aligned->aligned_sequence, "ACTA");  // 2M1I2M drops the inserted base
   ASSERT_EQ(aligned->insertions.size(), 1U);
   EXPECT_EQ(aligned->insertions.at(0), "22:G");
}

TEST(BamReader, rejectsNonBamMagic) {
   std::stringstream stream{gzipCompress("not a bam file at all")};
   EXPECT_ANY_THROW(BamReader{stream});
}
