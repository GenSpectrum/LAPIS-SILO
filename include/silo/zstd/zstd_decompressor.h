#pragma once

#include <string>
#include <string_view>

#include <zstd.h>

#include "zstd_context.h"
#include "zstd_dictionary.h"

namespace silo {

class ZstdDecompressor {
   ZstdDDictionary zstd_dictionary;
   ZstdDContext zstd_context;
   std::string buffer;

  public:
   explicit ZstdDecompressor(std::string_view dictionary_string);

   std::string_view decompress(const std::string& input);

   std::string_view decompress(const char* input_data, size_t input_length);
};

}  // namespace silo
