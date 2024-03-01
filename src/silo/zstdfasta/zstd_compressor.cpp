#include "silo/zstdfasta/zstd_compressor.h"

#include <string_view>
#include <utility>

#include <spdlog/spdlog.h>

namespace silo {

ZstdCompressor::ZstdCompressor() = default;

ZstdCompressor::ZstdCompressor(std::shared_ptr<silo::ZstdCDictionary> dictionary)
    : dictionary(std::move(dictionary)) {}

std::string_view ZstdCompressor::compress(const std::string& input) {
   return compress(input.data(), input.size());
}

std::string_view ZstdCompressor::compress(const char* input_data, size_t input_size) {
   if (ZSTD_compressBound(input_size) > buffer.size()) {
      buffer = std::string(ZSTD_compressBound(input_size), '\0');
   }
   const size_t size_or_error_code = ZSTD_compress_usingCDict(
      zstd_context.value, buffer.data(), buffer.size(), input_data, input_size, dictionary->value
   );
   if (ZSTD_isError(size_or_error_code)) {
      const std::string error_name = ZSTD_getErrorName(size_or_error_code);
      throw std::runtime_error(
         "Error '" + error_name + "' in dependency when decompressing using zstd."
      );
   }
   return {buffer.data(), size_or_error_code};
}

}  // namespace silo