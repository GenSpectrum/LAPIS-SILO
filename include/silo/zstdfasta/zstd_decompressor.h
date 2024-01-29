#pragma once

#include <string>
#include <string_view>

#include <zstd.h>

namespace silo {

class ZstdDecompressor {
   ZSTD_DDict* zstd_dictionary;
   ZSTD_DCtx* zstd_context;
   std::string buffer;

  public:
   ZstdDecompressor(ZstdDecompressor&& other) noexcept;
   ZstdDecompressor& operator=(ZstdDecompressor&& other) noexcept;

   ZstdDecompressor(const ZstdDecompressor& other) = delete;
   ZstdDecompressor& operator=(const ZstdDecompressor& other) = delete;
   virtual ~ZstdDecompressor();

   explicit ZstdDecompressor(std::string_view dictionary_string);

   std::string_view decompress(const std::string& input);

   std::string_view decompress(const char* input_data, size_t input_length);
};

}  // namespace silo
