#include "silo/zstdfasta/zstd_compressor.h"

#include <string_view>

namespace silo {

ZstdCompressor::~ZstdCompressor() {
   ZSTD_freeCCtx(zstd_context);
}

ZstdCompressor::ZstdCompressor(std::string_view dictionary_string) {
   size_bound = ZSTD_compressBound(dictionary_string.size());
   dictionary = std::make_shared<ZstdCompressDict>(dictionary_string, 2);
   zstd_context = ZSTD_createCCtx();
}

ZstdCompressor::ZstdCompressor(ZstdCompressor&& other) {
   this->zstd_context = std::exchange(other.zstd_context, nullptr);
   this->dictionary = std::exchange(other.dictionary, nullptr);
   this->size_bound = other.size_bound;
}

ZstdCompressor& ZstdCompressor::operator=(ZstdCompressor&& other) {
   std::swap(this->zstd_context, other.zstd_context);
   std::swap(this->dictionary, other.dictionary);
   std::swap(this->size_bound, other.size_bound);
   return *this;
}

ZstdCompressor::ZstdCompressor(const ZstdCompressor& other) {
   this->dictionary = other.dictionary;
   this->zstd_context = ZSTD_createCCtx();
   this->size_bound = other.size_bound;
}

ZstdCompressor& ZstdCompressor::operator=(const ZstdCompressor& other) {
   this->dictionary = other.dictionary;
   ZSTD_freeCCtx(zstd_context);
   this->zstd_context = ZSTD_createCCtx();
   this->size_bound = other.size_bound;
   return *this;
}

size_t ZstdCompressor::compress(const std::string& input, std::string& output) {
   return compress(input.data(), input.size(), output.data(), output.size());
}

size_t ZstdCompressor::compress(
   const char* input_data,
   size_t input_size,
   char* output_data,
   size_t output_size
) {
   size_t size_or_error_code = ZSTD_compress_usingCDict(
      zstd_context, output_data, output_size, input_data, input_size, dictionary->value
   );
   if (ZSTD_isError(size_or_error_code)) {
      throw std::runtime_error("Zstd compression failed.");
   }
   return size_or_error_code;
}

size_t ZstdCompressor::getSizeBound() const {
   return size_bound;
}

}  // namespace silo