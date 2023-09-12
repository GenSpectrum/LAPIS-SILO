#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <zstd.h>

namespace silo {

class ZstdCompressor {
   size_t size_bound;
   ZSTD_CDict* zstd_dictionary;
   ZSTD_CCtx* zstd_context;

  public:
   ZstdCompressor(const ZstdCompressor& other) = delete;
   ZstdCompressor& operator=(const ZstdCompressor& other) = delete;

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
