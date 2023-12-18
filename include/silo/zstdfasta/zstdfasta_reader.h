#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace silo {
struct ZstdDecompressor;

class ZstdFastaReader {
  private:
   std::ifstream in_file;
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
