#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <zstd.h>

#include "zstd_context.h"
#include "zstd_dictionary.h"

namespace silo {

class ZstdDecompressor {
   std::shared_ptr<ZstdDDictionary> zstd_dictionary;
   ZstdDContext zstd_context;

  public:
   explicit ZstdDecompressor(std::shared_ptr<ZstdDDictionary> zstd_dictionary);

   void decompress(const std::string& input, std::string& buffer);

   void decompress(const char* input_data, size_t input_length, std::string& buffer);
};

}  // namespace silo
