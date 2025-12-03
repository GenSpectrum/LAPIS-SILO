#include "silo/zstd/zstd_context.h"

#include <utility>

namespace silo {

ZstdCContext::ZstdCContext() {
   value = ZSTD_createCCtx();
}

ZstdCContext::ZstdCContext(silo::ZstdCContext&& other) noexcept {
   value = std::exchange(other.value, nullptr);
}

ZstdCContext& ZstdCContext::operator=(silo::ZstdCContext&& other) noexcept {
   std::swap(value, other.value);
   return *this;
}

ZstdCContext::~ZstdCContext() {
   ZSTD_freeCCtx(value);
}

ZstdDContext::ZstdDContext() {
   value = ZSTD_createDCtx();
}

ZstdDContext::ZstdDContext(silo::ZstdDContext&& other) noexcept {
   std::swap(value, other.value);
}

ZstdDContext& ZstdDContext::operator=(silo::ZstdDContext&& other) noexcept {
   std::swap(value, other.value);
   return *this;
}

ZstdDContext::~ZstdDContext() {
   ZSTD_freeDCtx(value);
}

}  // namespace silo
