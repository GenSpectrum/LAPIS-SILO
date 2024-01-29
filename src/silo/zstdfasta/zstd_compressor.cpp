#include "silo/zstdfasta/zstd_compressor.h"

#include <string_view>
#include <utility>

#include <spdlog/spdlog.h>

namespace silo {

ZstdCompressor::~ZstdCompressor() {
   ZSTD_freeCCtx(zstd_context);
}

ZstdCompressor::ZstdCompressor(std::string_view dictionary_string) {
   SPDLOG_TRACE("Creating ZstdCompressor object from dictionary");
   buffer = std::string(ZSTD_compressBound(dictionary_string.size()), '\0');
   dictionary = std::make_shared<ZstdCompressDict>(dictionary_string, 2);
   zstd_context = ZSTD_createCCtx();
}

ZstdCompressor::ZstdCompressor(ZstdCompressor&& other) noexcept {
   SPDLOG_TRACE("Moving ZstdCompressor object");
   this->zstd_context = std::exchange(other.zstd_context, nullptr);
   this->dictionary = std::exchange(other.dictionary, nullptr);
   this->buffer = std::move(other.buffer);
}

ZstdCompressor& ZstdCompressor::operator=(ZstdCompressor&& other) noexcept {
   SPDLOG_TRACE("Moving-assigning ZstdCompressor object");
   std::swap(this->zstd_context, other.zstd_context);
   std::swap(this->dictionary, other.dictionary);
   std::swap(this->buffer, other.buffer);
   return *this;
}

ZstdCompressor::ZstdCompressor(const ZstdCompressor& other) {
   SPDLOG_TRACE("Copying ZstdCompressor object");
   this->dictionary = other.dictionary;
   this->zstd_context = ZSTD_createCCtx();
   this->buffer = std::string(other.buffer.size(), '\0');
}

ZstdCompressor& ZstdCompressor::operator=(const ZstdCompressor& other) {
   SPDLOG_TRACE("Copying assigning ZstdCompressor object");
   if (this == &other) {
      return *this;
   }

   this->dictionary = other.dictionary;
   ZSTD_freeCCtx(zstd_context);
   this->zstd_context = ZSTD_createCCtx();
   this->buffer = std::string(other.buffer.size(), '\0');
   return *this;
}

std::string_view ZstdCompressor::compress(const std::string& input) {
   return compress(input.data(), input.size());
}

std::string_view ZstdCompressor::compress(const char* input_data, size_t input_size) {
   if (ZSTD_compressBound(input_size) > buffer.size()) {
      buffer = std::string(ZSTD_compressBound(input_size), '\0');
   }
   const size_t size_or_error_code = ZSTD_compress_usingCDict(
      zstd_context, buffer.data(), buffer.size(), input_data, input_size, dictionary->value
   );
   if (ZSTD_isError(size_or_error_code)) {
      const std::string error_name = ZSTD_getErrorName(size_or_error_code);
      throw std::runtime_error(
         "Error '" + error_name + "' in dependency when decompressing using zstd."
      );
   }
   return std::string_view(buffer.data(), size_or_error_code);
}

}  // namespace silo