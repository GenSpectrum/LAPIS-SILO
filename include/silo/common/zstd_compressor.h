#ifndef SILO_ZSTD_COMPRESSOR_H
#define SILO_ZSTD_COMPRESSOR_H

#include <cstddef>
#include <string>

#include <zstd.h>

namespace silo {

class ZstdCompressor {
   ZSTD_CDict* zstd_dictionary;
   ZSTD_CCtx* zstd_context;

  public:
   ZstdCompressor(const ZstdCompressor& other) = delete;
   ZstdCompressor(ZstdCompressor&& other) = delete;
   ZstdCompressor operator=(const ZstdCompressor& other) = delete;
   ZstdCompressor operator=(ZstdCompressor&& other) = delete;
   virtual ~ZstdCompressor();

   explicit ZstdCompressor(std::string dictionary_string);

   size_t compress(const std::string& input, std::string& output);
};

}  // namespace silo

#endif  // SILO_ZSTD_COMPRESSOR_H
