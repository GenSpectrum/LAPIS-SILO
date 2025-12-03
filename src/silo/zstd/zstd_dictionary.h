#pragma once

#include <string_view>

#include <zstd.h>

namespace silo {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class ZstdCompressor;
class ZstdDecompressor;
}  // namespace silo

namespace silo {

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
   explicit ZstdDDictionary(std::string_view data);

   ZstdDDictionary(const ZstdDDictionary& other) = delete;
   ZstdDDictionary& operator=(const ZstdDDictionary& other) = delete;

   ZstdDDictionary(ZstdDDictionary&& other) noexcept;
   ZstdDDictionary& operator=(ZstdDDictionary&& other) noexcept;

   ~ZstdDDictionary();
};

}  // namespace silo