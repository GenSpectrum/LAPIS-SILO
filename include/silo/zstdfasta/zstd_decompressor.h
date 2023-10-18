#pragma once

#include <string>
#include <string_view>

#include <zstd.h>

namespace silo {

class ZstdDecompressor {
   ZSTD_DDict* zstd_dictionary;
   ZSTD_DCtx* zstd_context;

  public:
   ZstdDecompressor(ZstdDecompressor&& other);
   ZstdDecompressor& operator=(ZstdDecompressor&& other);

   ZstdDecompressor(const ZstdDecompressor& other) = delete;
   ZstdDecompressor& operator=(const ZstdDecompressor& other) = delete;
   virtual ~ZstdDecompressor();

   explicit ZstdDecompressor(std::string_view dictionary_string);

   void decompress(const std::string& input, std::string& output);

   void decompress(
      const char* input_data,
      size_t input_length,
      char* output_data,
      size_t output_length
   );
};

}  // namespace silo
