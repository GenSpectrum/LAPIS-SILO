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
   size_t size_bound;
   std::shared_ptr<ZstdCompressDict> dictionary;
   ZSTD_CCtx* zstd_context;

  public:
   ZstdCompressor(const ZstdCompressor& other);
   ZstdCompressor& operator=(const ZstdCompressor& other);

   ZstdCompressor(ZstdCompressor&& other);
   ZstdCompressor& operator=(ZstdCompressor&& other);

   virtual ~ZstdCompressor();

   explicit ZstdCompressor(std::string_view dictionary_string);

   size_t compress(const std::string& input, std::string& output);
   size_t compress(
      const char* input_data,
      size_t input_size,
      char* output_data,
      size_t output_size
   );

   size_t getSizeBound() const;
};

}  // namespace silo
