#pragma once

#include <string_view>

#include <zstd.h>

namespace silo {

class ZstdCompressor;
class ZstdDecompressor;

class ZstdCDictionary final {
   friend class ZstdCompressor;
   ZSTD_CDict* value;

  public:
   ZstdCDictionary(std::string_view data, int compression_level);

   ZstdCDictionary(const ZstdCDictionary& other) = delete;
   ZstdCDictionary& operator=(const ZstdCDictionary& other) = delete;

   ZstdCDictionary(ZstdCDictionary&& other) noexcept;
   ZstdCDictionary& operator=(ZstdCDictionary&& other) noexcept;

   ~ZstdCDictionary();
};

class ZstdDDictionary final {
   friend class ZstdDecompressor;
   ZSTD_DDict* value;

  public:
   ZstdDDictionary(std::string_view data);

   ZstdDDictionary(const ZstdDDictionary& other) = delete;
   ZstdDDictionary& operator=(const ZstdDDictionary& other) = delete;

   ZstdDDictionary(ZstdDDictionary&& other) noexcept;
   ZstdDDictionary& operator=(ZstdDDictionary&& other) noexcept;

   ~ZstdDDictionary();
};

}  // namespace silo