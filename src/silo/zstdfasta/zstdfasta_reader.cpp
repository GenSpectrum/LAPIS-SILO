#include "silo/zstdfasta/zstdfasta_reader.h"

#include <fstream>
#include <stdexcept>

#include "silo/common/fasta_format_exception.h"
#include "silo/zstdfasta/zstd_decompressor.h"

silo::ZstdFastaReader::ZstdFastaReader(
   const std::filesystem::path& in_file_name,
   std::string_view compression_dict
)
    : decompressor(std::make_unique<ZstdDecompressor>(compression_dict)) {
   in_file = std::ifstream(in_file_name);
   if (!in_file) {
      throw std::runtime_error("Could not open file reader for file: " + in_file_name.string());
   }
}

std::optional<std::string> silo::ZstdFastaReader::nextKey() {
   std::string key_with_prefix;
   if (!getline(in_file, key_with_prefix)) {
      return std::nullopt;
   }

   if (key_with_prefix.empty() || key_with_prefix.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + key_with_prefix);
   }

   return key_with_prefix.substr(1);
}

std::optional<std::string> silo::ZstdFastaReader::nextSkipGenome() {
   auto key = nextKey();

   if (!key) {
      return std::nullopt;
   }

   std::string bytestream_length_str;
   if (!getline(in_file, bytestream_length_str)) {
      throw FastaFormatException("Missing bytestream length in line following key: " + *key);
   }
   const size_t bytestream_length = std::stoul(bytestream_length_str);

   in_file.ignore(static_cast<std::streamsize>(bytestream_length));
   return key;
}

std::optional<std::string> silo::ZstdFastaReader::nextCompressed(std::string& compressed_genome) {
   auto key = nextKey();
   if (!key) {
      return std::nullopt;
   }

   std::string bytestream_length_str;
   if (!getline(in_file, bytestream_length_str)) {
      throw FastaFormatException("Missing bytestream length in line following key: " + *key);
   }
   const size_t bytestream_length = std::stoul(bytestream_length_str);

   compressed_genome.resize(bytestream_length);
   in_file.read(compressed_genome.data(), static_cast<std::streamsize>(compressed_genome.size()));
   in_file.ignore(1);
   return key;
}

std::optional<std::string> silo::ZstdFastaReader::next(std::string& genome) {
   std::string compressed_buffer;
   auto key = nextCompressed(compressed_buffer);

   if (!key) {
      return std::nullopt;
   }
   genome = decompressor->decompress(compressed_buffer);
   return key;
}

void silo::ZstdFastaReader::reset() {
   in_file.clear();                  // clear fail and eof bits
   in_file.seekg(0, std::ios::beg);  // g pointer back to the start
}
