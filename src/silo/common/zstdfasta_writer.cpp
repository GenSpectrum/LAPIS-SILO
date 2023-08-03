#include "silo/common/zstdfasta_writer.h"

#include <cstddef>
#include <filesystem>
#include <stdexcept>

#include "silo/common/zstd_compressor.h"

silo::ZstdFastaWriter::ZstdFastaWriter(
   const std::filesystem::path& out_file,
   std::string_view compression_dict
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

   buffer = std::string(compressor->getSizeBound(), '\0');
}

silo::ZstdFastaWriter::ZstdFastaWriter(
   const std::filesystem::path& out_file,
   std::string_view compression_dict,
   const std::string& default_sequence_
)
    : ZstdFastaWriter(out_file, compression_dict) {
   const size_t compressed_length = compressor->compress(default_sequence_, buffer);
   default_sequence = buffer;
   default_sequence->resize(compressed_length);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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

void silo::ZstdFastaWriter::writeDefault(const std::string& key) {
   if (default_sequence == std::nullopt) {
      throw std::runtime_error(
         "Tried to write default sequence, when non was provided in the ZstdFastaWriter constructor"
      );
   }
   outStream << '>' << key << '\n'
             << std::to_string(default_sequence->size()) << '\n'
             << *default_sequence << '\n';
}
