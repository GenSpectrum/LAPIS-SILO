#ifndef SILO_ZSTDFASTA_WRITER_H
#define SILO_ZSTDFASTA_WRITER_H

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "silo/common/input_stream_wrapper.h"
#include "silo/common/zstd_compressor.h"

namespace silo {
class ZstdFastaWriter {
  private:
   std::ofstream outStream;
   std::unique_ptr<ZstdCompressor> compressor;
   std::string buffer;
   std::optional<std::string> default_sequence;

  public:
   explicit ZstdFastaWriter(
      const std::filesystem::path& out_file_name,
      std::string_view compression_dict
   );

   explicit ZstdFastaWriter(
      const std::filesystem::path& out_file_name,
      std::string_view compression_dict,
      const std::string& default_sequence_
   );

   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   void write(const std::string& key, const std::string& genome);

   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   void writeRaw(const std::string& key, const std::string& compressed_genome);

   // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
   void writeDefault(const std::string& key);
};
}  // namespace silo

#endif  // SILO_ZSTDFASTA_WRITER_H
