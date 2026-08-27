#pragma once

#include <cstddef>
#include <istream>
#include <vector>

#include <zlib.h>

namespace rhydb::append::bam {

/// Streaming decompressor for the BGZF container that BAM files use.
///
/// BGZF is a sequence of concatenated gzip members (each an independently
/// deflated block), so a forward-only decode is just multi-member gzip inflate;
/// we do not need to parse the per-block BSIZE subfields. (Random access via the
/// BAI index and virtual offsets would need them, but ingestion only scans
/// forward.) The reader keeps memory bounded by inflating into a fixed buffer
/// and serving byte ranges out of it on demand.
class BgzfReader {
  public:
   explicit BgzfReader(std::istream& compressed_input);
   ~BgzfReader();

   BgzfReader(const BgzfReader&) = delete;
   BgzfReader& operator=(const BgzfReader&) = delete;
   BgzfReader(BgzfReader&&) = delete;
   BgzfReader& operator=(BgzfReader&&) = delete;

   /// Copy up to `length` decompressed bytes into `destination`. Returns the
   /// number of bytes actually produced; a short count (including 0) means the
   /// end of the stream was reached. Throws BamException on corrupt input.
   size_t read(char* destination, size_t length);

   /// Fill `destination` with exactly `length` bytes, or throw BamException if
   /// the stream ends first (a truncated file).
   void readExact(char* destination, size_t length);

   /// Whether all input has been consumed and fully decompressed.
   [[nodiscard]] bool eof() const { return stream_finished && output_begin >= output_end; }

  private:
   /// Refill the output buffer from the compressed stream. Returns false once the
   /// stream is exhausted.
   bool fillOutput();

   std::istream* input;
   z_stream zstream;
   std::vector<char> input_buffer;
   std::vector<char> output_buffer;
   size_t output_begin = 0;
   size_t output_end = 0;
   bool input_eof = false;
   bool stream_finished = false;
};

}  // namespace rhydb::append::bam
