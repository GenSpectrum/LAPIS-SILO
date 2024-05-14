#pragma once

#include <zstd.h>

namespace silo {

class ZstdCompressor;

class ZstdCContext final {
   friend class ZstdCompressor;
   ZSTD_CCtx_s* value;

  public:
   ZstdCContext();

   ZstdCContext(const ZstdCContext& other) = delete;
   ZstdCContext& operator=(const ZstdCContext& other) = delete;

   ZstdCContext(ZstdCContext&& other) noexcept;
   ZstdCContext& operator=(ZstdCContext&& other) noexcept;

   ~ZstdCContext();
};

class ZstdDContext final {
   friend class ZstdDecompressor;
   ZSTD_DCtx_s* value;

  public:
   ZstdDContext();

   ZstdDContext(const ZstdDContext& other) = delete;
   ZstdDContext& operator=(const ZstdDContext& other) = delete;

   ZstdDContext(ZstdDContext&& other) noexcept;
   ZstdDContext& operator=(ZstdDContext&& other) noexcept;

   ~ZstdDContext();
};

}  // namespace silo
