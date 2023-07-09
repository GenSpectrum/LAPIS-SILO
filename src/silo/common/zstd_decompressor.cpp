#include "silo/common/zstd_decompressor.h"

#include <string>

namespace silo {

ZstdDecompressor::~ZstdDecompressor() {
   ZSTD_freeDDict(zstd_dictionary);
   ZSTD_freeDCtx(zstd_context);
}

ZstdDecompressor::ZstdDecompressor(std::string_view dictionary_string) {
   zstd_dictionary = ZSTD_createDDict(dictionary_string.data(), dictionary_string.length());
   zstd_context = ZSTD_createDCtx();
}

void ZstdDecompressor::decompress(const std::string& input, std::string& output) {
   ZSTD_decompress_usingDDict(
      zstd_context, output.data(), output.length(), input.data(), input.size(), zstd_dictionary
   );
}

}  // namespace silo