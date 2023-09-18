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

ZstdDecompressor::ZstdDecompressor(ZstdDecompressor&& other) {
   this->zstd_context = std::exchange(other.zstd_context, nullptr);
   this->zstd_dictionary = std::exchange(other.zstd_dictionary, nullptr);
}

ZstdDecompressor& ZstdDecompressor::operator=(ZstdDecompressor&& other) {
   std::swap(this->zstd_context, other.zstd_context);
   std::swap(this->zstd_dictionary, other.zstd_dictionary);
   return *this;
}

void ZstdDecompressor::decompress(const std::string& input, std::string& output) {
   ZSTD_decompress_usingDDict(
      zstd_context, output.data(), output.length(), input.data(), input.size(), zstd_dictionary
   );
}

void ZstdDecompressor::decompress(
   const char* input_data,
   size_t input_length,
   char* output_data,
   size_t output_length
) {
   ZSTD_decompress_usingDDict(
      zstd_context, output_data, output_length, input_data, input_length, zstd_dictionary
   );
}

}  // namespace silo