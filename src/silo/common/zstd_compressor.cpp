#include "silo/common/zstd_compressor.h"

#include <string_view>

namespace silo {

ZstdCompressor::~ZstdCompressor() {
   ZSTD_freeCDict(zstd_dictionary);
   ZSTD_freeCCtx(zstd_context);
}

ZstdCompressor::ZstdCompressor(std::string_view dictionary_string) {
   size_bound = ZSTD_compressBound(dictionary_string.size());
   zstd_dictionary = ZSTD_createCDict(dictionary_string.data(), dictionary_string.length(), 2);
   zstd_context = ZSTD_createCCtx();
}

size_t ZstdCompressor::compress(const std::string& input, std::string& output) {
   return ZSTD_compress_usingCDict(
      zstd_context, output.data(), output.size(), input.data(), input.size(), zstd_dictionary
   );
}

size_t ZstdCompressor::getSizeBound() const {
   return size_bound;
}

}  // namespace silo