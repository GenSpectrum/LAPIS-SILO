#include "silo/common/zstdfasta_writer.h"

#include <cstddef>
#include <filesystem>
#include <stdexcept>

#include "silo/common/zstd_compressor.h"

silo::ZstdFastaWriter::ZstdFastaWriter(
   const std::filesystem::path& out_file,
   const std::string& compression_dict
)
    : compressor(std::make_unique<ZstdCompressor>(compression_dict)) {
   if (!exists(out_file)) {
      if (!exists(out_file.parent_path())) {
         if (!create_directories(out_file.parent_path())) {
            throw std::runtime_error(
               "Could not create file " + std::string{out_file.relative_path()}
            );
         }
      }
   }
   outStream = std::ofstream(out_file.relative_path());

   const size_t size_bound = ZSTD_compressBound(compression_dict.size());
   buffer = std::string(size_bound, '\0');
}

void silo::ZstdFastaWriter::write(const std::string& key, const std::string& genome) {
   const size_t compressed_length = compressor->compress(genome, buffer);

   outStream << '>' << key << '\n' << std::to_string(compressed_length) << '\n';
   outStream.write(buffer.data(), static_cast<std::streamsize>(compressed_length));
   outStream << '\n';
}

void silo::ZstdFastaWriter::writeRaw(const std::string& key, const std::string& compressed_genome) {
   outStream << '>' << key << '\n'
             << std::to_string(compressed_genome.size()) << '\n'
             << compressed_genome << '\n';
}
