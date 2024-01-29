#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include <zstd.h>

namespace silo {

struct ZstdCompressDict {
   ZSTD_CDict* value;

   ZstdCompressDict(std::string_view data, uint32_t compression_level) {
      value = ZSTD_createCDict(data.data(), data.size(), compression_level);
   }

   ~ZstdCompressDict() { ZSTD_freeCDict(value); }
};

class ZstdCompressor {
   std::string buffer;
   std::shared_ptr<ZstdCompressDict> dictionary;
   ZSTD_CCtx* zstd_context;

  public:
   ZstdCompressor(const ZstdCompressor& other);
   ZstdCompressor& operator=(const ZstdCompressor& other);

   ZstdCompressor(ZstdCompressor&& other) noexcept;
   ZstdCompressor& operator=(ZstdCompressor&& other) noexcept;

   virtual ~ZstdCompressor();

   explicit ZstdCompressor(std::string_view dictionary_string);

   std::string_view compress(const std::string& input);
   std::string_view compress(const char* input_data, size_t input_size);
};

}  // namespace silo
