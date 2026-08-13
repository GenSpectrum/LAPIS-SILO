#include "rhydb/zstd/zstd_context.h"

#include <utility>

namespace rhydb {

ZstdCContext::ZstdCContext() {
   value = ZSTD_createCCtx();
}

ZstdCContext::ZstdCContext(rhydb::ZstdCContext&& other) noexcept {
   value = std::exchange(other.value, nullptr);
}

ZstdCContext& ZstdCContext::operator=(rhydb::ZstdCContext&& other) noexcept {
   std::swap(value, other.value);
   return *this;
}

ZstdCContext::~ZstdCContext() {
   ZSTD_freeCCtx(value);
}

ZstdDContext::ZstdDContext() {
   value = ZSTD_createDCtx();
}

ZstdDContext::ZstdDContext(rhydb::ZstdDContext&& other) noexcept {
   value = std::exchange(other.value, nullptr);
}

ZstdDContext& ZstdDContext::operator=(rhydb::ZstdDContext&& other) noexcept {
   std::swap(value, other.value);
   return *this;
}

ZstdDContext::~ZstdDContext() {
   ZSTD_freeDCtx(value);
}

}  // namespace rhydb
