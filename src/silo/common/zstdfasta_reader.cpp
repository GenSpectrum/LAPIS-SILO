#include "silo/common/zstdfasta_reader.h"

#include "silo/common/fasta_format_exception.h"

silo::ZstdFastaReader::ZstdFastaReader(
   const std::filesystem::path& in_file_name,
   const std::string& compression_dict
)
    : in_file(in_file_name),
      decompressor(std::make_unique<ZstdDecompressor>(compression_dict)) {
   genome_buffer = std::string(compression_dict.length(), '\0');
}

bool silo::ZstdFastaReader::populateKey(std::string& key) {
   std::string key_with_prefix;
   if (!getline(in_file.getInputStream(), key_with_prefix)) {
      return false;
   }

   if (key_with_prefix.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + key_with_prefix);
   }

   key = key_with_prefix.substr(1);
   return true;
}

bool silo::ZstdFastaReader::nextKey(std::string& key) {
   auto key_was_read = populateKey(key);

   std::string bytestream_length_str;
   if (!getline(in_file.getInputStream(), bytestream_length_str)) {
      throw FastaFormatException("Missing bytestream length in line following key: " + key);
   }
   const size_t bytestream_length = std::stoul(bytestream_length_str);

   in_file.getInputStream().ignore(static_cast<std::streamsize>(bytestream_length));
   return key_was_read;
}

bool silo::ZstdFastaReader::nextCompressed(std::string& key, std::string& compressed_genome) {
   auto key_was_read = populateKey(key);
   if (!key_was_read) {
      return false;
   }

   std::string bytestream_length_str;
   if (!getline(in_file.getInputStream(), bytestream_length_str)) {
      throw FastaFormatException("Missing bytestream length in line following key: " + key);
   }
   const size_t bytestream_length = std::stoul(bytestream_length_str);

   compressed_genome.resize(bytestream_length);
   in_file.getInputStream().read(
      compressed_genome.data(), static_cast<std::streamsize>(compressed_genome.size())
   );
   in_file.getInputStream().ignore(1);
   return true;
}

bool silo::ZstdFastaReader::next(std::string& key, std::string& genome) {
   std::string compressed_buffer;
   if (!nextCompressed(key, compressed_buffer)) {
      return false;
   }
   decompressor->decompress(compressed_buffer, genome_buffer);
   genome = genome_buffer;
   return true;
}

void silo::ZstdFastaReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
