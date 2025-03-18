#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include <zstd.h>

#include "silo/zstd/zstd_context.h"
#include "silo/zstd/zstd_dictionary.h"

namespace silo {

class ZstdCompressor {
   std::string buffer;
   std::shared_ptr<ZstdCDictionary> dictionary;
   ZstdCContext zstd_context;

   ZstdCompressor() = delete;

  public:
   explicit ZstdCompressor(std::shared_ptr<ZstdCDictionary> dictionary);

   std::string_view compress(const std::string& input);
   std::string_view compress(const char* input_data, size_t input_size);
};

}  // namespace silo
