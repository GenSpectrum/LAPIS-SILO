#ifndef SILO_ZSTDFASTA_WRITER_H
#define SILO_ZSTDFASTA_WRITER_H

#include <filesystem>
#include <iostream>

#include <zstd.h>

#include "silo/common/input_stream_wrapper.h"

namespace silo {
class ZstdFastaWriter {
  private:
   std::ofstream outStream;
   ZSTD_CDict* zstd_dictionary;
   ZSTD_CCtx* zstd_context;
   std::string buffer;

  public:
   explicit ZstdFastaWriter(
      const std::filesystem::path& out_file_name,
      const std::string& compression_dict
   );

   void write(const std::string& key, const std::string& genome);

   void writeRaw(const std::string& key, const std::string& compressed_genome);
};
}  // namespace silo

#endif  // SILO_ZSTDFASTA_WRITER_H