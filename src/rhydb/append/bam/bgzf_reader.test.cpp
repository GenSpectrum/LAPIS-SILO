#include "rhydb/append/bam/bgzf_reader.h"

#include <array>
#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <zlib.h>

#include "rhydb/append/bam/bam_exception.h"

using rhydb::append::bam::BamException;
using rhydb::append::bam::BgzfReader;

namespace {

/// Compress `data` into a single standalone gzip member. A BGZF block is exactly
/// such a member, so this exercises the reader's real decode path; concatenating
/// the output of several calls produces the multi-member layout of a BAM file.
std::string gzipCompress(std::string_view data) {
   z_stream stream{};
   const int init =
      deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
   EXPECT_EQ(init, Z_OK);
   std::string out;
   out.resize(deflateBound(&stream, data.size()));
   stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
   stream.avail_in = static_cast<uInt>(data.size());
   stream.next_out = reinterpret_cast<Bytef*>(out.data());
   stream.avail_out = static_cast<uInt>(out.size());
   const int status = deflate(&stream, Z_FINISH);
   EXPECT_EQ(status, Z_STREAM_END);
   out.resize(out.size() - stream.avail_out);
   deflateEnd(&stream);
   return out;
}

std::string decompressAll(std::string compressed) {
   std::stringstream stream{std::move(compressed)};
   BgzfReader reader{stream};
   std::string out;
   std::array<char, 7> buffer{};  // deliberately awkward size to cross buffer boundaries
   while (true) {
      const size_t got = reader.read(buffer.data(), buffer.size());
      if (got == 0) {
         break;
      }
      out.append(buffer.data(), got);
   }
   EXPECT_TRUE(reader.eof());
   return out;
}

}  // namespace

TEST(BgzfReader, decompressesSingleMember) {
   const std::string original = "Hello, BAM world!";
   EXPECT_EQ(decompressAll(gzipCompress(original)), original);
}

TEST(BgzfReader, decompressesConcatenatedMembers) {
   // BGZF files are many gzip members back to back; the reader must span them.
   const std::string first = "first block of data;";
   const std::string second = "second block of data;";
   const std::string third = "third block.";
   EXPECT_EQ(
      decompressAll(gzipCompress(first) + gzipCompress(second) + gzipCompress(third)),
      first + second + third
   );
}

TEST(BgzfReader, decompressesPayloadLargerThanOutputBuffer) {
   std::string original;
   original.reserve(500000);
   for (int i = 0; i < 50000; ++i) {
      original += "ACGTACGTAC";
   }
   EXPECT_EQ(decompressAll(gzipCompress(original)), original);
}

TEST(BgzfReader, emptyStreamYieldsNothing) {
   std::stringstream empty;
   BgzfReader reader{empty};
   std::array<char, 8> buffer{};
   EXPECT_EQ(reader.read(buffer.data(), buffer.size()), 0U);
   EXPECT_TRUE(reader.eof());
}

TEST(BgzfReader, readExactThrowsPastEndOfStream) {
   std::stringstream stream{gzipCompress("AB")};
   BgzfReader reader{stream};
   std::array<char, 5> buffer{};
   EXPECT_THROW(reader.readExact(buffer.data(), buffer.size()), BamException);
}
