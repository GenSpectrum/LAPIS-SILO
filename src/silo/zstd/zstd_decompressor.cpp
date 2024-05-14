#include "silo/zstd/zstd_decompressor.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <cassert>

namespace silo {

ZstdDecompressor::ZstdDecompressor(std::string_view dictionary_string)
    : zstd_dictionary(ZstdDDictionary(dictionary_string)) {}

void ZstdDecompressor::decompress(const std::string& input, std::string& buffer) {
   decompress(input.data(), input.size(), buffer);
}

void ZstdDecompressor::decompress(
   const char* input_data,
   size_t input_length,
   std::string& buffer
) {
   const size_t uncompressed_size = ZSTD_getFrameContentSize(input_data, input_length);
   if (uncompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
      throw std::runtime_error(fmt::format(
         "ZSTD_Error: Cannot decompress data with unknown size (getFrameContentSize == "
         "UNKNOWN) for compressed data of length {}",
         input_length
      ));
   }
   if (uncompressed_size == ZSTD_CONTENTSIZE_ERROR) {
      throw std::runtime_error(fmt::format(
         "ZSTD_Error: Error in dependency, when getting decompressed size for compressed data of "
         "length {}"
         "(getFrameContentSize)",
         input_length
      ));
   }
   buffer.resize(uncompressed_size);
   auto size_or_error_code = ZSTD_decompress_usingDDict(
      zstd_context.value,
      buffer.data(),
      buffer.size(),
      input_data,
      input_length,
      zstd_dictionary.value
   );
   if (ZSTD_isError(size_or_error_code)) {
      const std::string error_name = ZSTD_getErrorName(size_or_error_code);
      throw std::runtime_error(
         fmt::format("Error '{}' in dependency when decompressing using zstd", error_name)
      );
   }
   assert(uncompressed_size == size_or_error_code);
}

}  // namespace silo