#ifndef SILO_ZSTDFASTA_READER_H
#define SILO_ZSTDFASTA_READER_H

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "silo/common/input_stream_wrapper.h"
#include "silo/common/zstd_decompressor.h"

namespace silo {
class ZstdFastaReader {
  private:
   silo::InputStreamWrapper in_file;
   std::unique_ptr<silo::ZstdDecompressor> decompressor;
   std::string genome_buffer;

   std::optional<std::string> nextKey();

  public:
   explicit ZstdFastaReader(
      const std::filesystem::path& in_file_name,
      std::string_view compression_dict
   );

   std::optional<std::string> nextSkipGenome();

   std::optional<std::string> next(std::string& genome);

   std::optional<std::string> nextCompressed(std::string& compressed_genome);

   void reset();
};
}  // namespace silo

#endif  // SILO_ZSTDFASTA_READER_H
