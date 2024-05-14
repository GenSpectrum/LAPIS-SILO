#include "silo/zstd/zstd_dictionary.h"

#include <utility>

namespace silo {

ZstdCDictionary::ZstdCDictionary(std::string_view data, int compression_level) {
   value = ZSTD_createCDict(data.data(), data.size(), compression_level);
}

ZstdCDictionary::ZstdCDictionary(ZstdCDictionary&& other) noexcept {
   value = std::exchange(other.value, nullptr);
}

ZstdCDictionary& ZstdCDictionary::operator=(ZstdCDictionary&& other) noexcept {
   std::swap(this->value, other.value);
   return *this;
}

ZstdCDictionary::~ZstdCDictionary() {
   ZSTD_freeCDict(value);
}

ZstdDDictionary::ZstdDDictionary(std::string_view data) {
   value = ZSTD_createDDict(data.data(), data.size());
}

ZstdDDictionary::ZstdDDictionary(ZstdDDictionary&& other) noexcept {
   value = std::exchange(other.value, nullptr);
}

ZstdDDictionary& ZstdDDictionary::operator=(ZstdDDictionary&& other) noexcept {
   std::swap(this->value, other.value);
   return *this;
}

ZstdDDictionary::~ZstdDDictionary() {
   ZSTD_freeDDict(value);
}

}  // namespace silo