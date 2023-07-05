#ifndef SILO_ZSTD_DECOMPRESSOR_H
#define SILO_ZSTD_DECOMPRESSOR_H

#include <string>

#include <zstd.h>

namespace silo {

class ZstdDecompressor {
   ZSTD_DDict* zstd_dictionary;
   ZSTD_DCtx* zstd_context;

  public:
   ZstdDecompressor(const ZstdDecompressor& other) = delete;
   ZstdDecompressor(ZstdDecompressor&& other) = delete;
   ZstdDecompressor operator=(const ZstdDecompressor& other) = delete;
   ZstdDecompressor operator=(ZstdDecompressor&& other) = delete;
   virtual ~ZstdDecompressor();

   explicit ZstdDecompressor(std::string dictionary_string);

   void decompress(const std::string& input, std::string& output);
};

}  // namespace silo

#endif  // SILO_ZSTD_DECOMPRESSOR_H
