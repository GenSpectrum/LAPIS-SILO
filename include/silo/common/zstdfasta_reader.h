#ifndef SILO_ZSTDFASTA_READER_H
#define SILO_ZSTDFASTA_READER_H

#include <filesystem>
#include <iostream>

#include <zstd.h>

#include "silo/common/input_stream_wrapper.h"

namespace silo {
class ZstdFastaReader {
  private:
   silo::InputStreamWrapper in_file;
   ZSTD_DDict* zstd_dictionary;
   ZSTD_DCtx* zstd_context;
   std::string genome_buffer;

   bool populateKey(std::string& key);

  public:
   explicit ZstdFastaReader(
      const std::filesystem::path& in_file_name,
      const std::string& compression_dict
   );

   bool nextKey(std::string& key);

   bool next(std::string& key, std::string& genome);

   bool nextCompressed(std::string& key, std::string& compressed_genome);

   void reset();
};
}  // namespace silo

#endif  // SILO_ZSTDFASTA_READER_H
