#include "rhydb/append/bam/bgzf_reader.h"

#include <algorithm>
#include <cstring>

#include "rhydb/append/bam/bam_exception.h"

namespace rhydb::append::bam {

namespace {
constexpr size_t INPUT_CHUNK_SIZE = 64 * 1024;
constexpr size_t OUTPUT_CHUNK_SIZE = 128 * 1024;
/// 15-bit window plus the +16 that tells zlib the input carries a gzip
/// header/trailer (as each BGZF block does).
constexpr int GZIP_WINDOW_BITS = 15 + 16;
}  // namespace

BgzfReader::BgzfReader(std::istream& compressed_input)
    : input(&compressed_input),
      input_buffer(INPUT_CHUNK_SIZE),
      output_buffer(OUTPUT_CHUNK_SIZE) {
   zstream = {};
   if (inflateInit2(&zstream, GZIP_WINDOW_BITS) != Z_OK) {
      throw BamException("failed to initialize zlib inflate for BGZF decompression");
   }
}

BgzfReader::~BgzfReader() {
   inflateEnd(&zstream);
}

bool BgzfReader::fillOutput() {
   while (true) {
      if (zstream.avail_in == 0) {
         if (input_eof) {
            stream_finished = true;
            return false;
         }
         input->read(input_buffer.data(), static_cast<std::streamsize>(input_buffer.size()));
         const std::streamsize bytes_read = input->gcount();
         if (bytes_read <= 0) {
            input_eof = true;
            continue;
         }
         if (input->eof()) {
            input_eof = true;
         }
         zstream.next_in = reinterpret_cast<Bytef*>(input_buffer.data());
         zstream.avail_in = static_cast<uInt>(bytes_read);
      }

      zstream.next_out = reinterpret_cast<Bytef*>(output_buffer.data());
      zstream.avail_out = static_cast<uInt>(output_buffer.size());
      const int status = inflate(&zstream, Z_NO_FLUSH);
      const size_t produced = output_buffer.size() - zstream.avail_out;

      if (status == Z_STREAM_END) {
         // One gzip member (BGZF block) finished. next_in/avail_in still point at
         // the start of the following member, so reset and keep going.
         if (inflateReset(&zstream) != Z_OK) {
            throw BamException("failed to reset zlib inflate between BGZF blocks");
         }
         if (produced > 0) {
            output_begin = 0;
            output_end = produced;
            return true;
         }
         continue;  // empty member (e.g. the 28-byte BGZF EOF marker)
      }
      if (status != Z_OK && status != Z_BUF_ERROR) {
         throw BamException(
            "zlib inflate failed while decompressing BGZF input: {}",
            zstream.msg != nullptr ? zstream.msg : "unknown error"
         );
      }
      if (produced > 0) {
         output_begin = 0;
         output_end = produced;
         return true;
      }
      // Made no progress. If there is no more input to feed, the stream is done
      // (or truncated); otherwise loop to pull the next input chunk.
      if (zstream.avail_in == 0 && input_eof) {
         stream_finished = true;
         return false;
      }
   }
}

size_t BgzfReader::read(char* destination, size_t length) {
   size_t total = 0;
   while (total < length) {
      if (output_begin < output_end) {
         const size_t take = std::min(length - total, output_end - output_begin);
         std::memcpy(destination + total, output_buffer.data() + output_begin, take);
         output_begin += take;
         total += take;
         continue;
      }
      if (stream_finished) {
         break;
      }
      if (!fillOutput()) {
         break;
      }
   }
   return total;
}

void BgzfReader::readExact(char* destination, size_t length) {
   const size_t got = read(destination, length);
   if (got != length) {
      throw BamException("unexpected end of BGZF stream: wanted {} bytes, got {}", length, got);
   }
}

}  // namespace rhydb::append::bam
