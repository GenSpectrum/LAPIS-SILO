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

  public:
   explicit ZstdDecompressor(std::string_view dictionary_string);

   void decompress(const std::string& input, std::string& buffer);

   void decompress(const char* input_data, size_t input_length, std::string& buffer);
};

}  // namespace silo
