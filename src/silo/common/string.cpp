#include "silo/common/string.h"

#include <cstring>
#include <string_view>

#include "silo/common/bidirectional_map.h"
#include "silo/common/types.h"

namespace silo::common {

template <size_t I>
String<I>::String(const std::string& string, BidirectionalMap<std::string>& dictionary) {
   const uint32_t length = string.length();
   *reinterpret_cast<uint32_t*>(data.data()) = length;
   if (length <= I) {
      memcpy(data.data() + 4, string.data(), length);
      memset(data.data() + 4 + length, '\0', I - length);
   } else {
      const Idx idx = dictionary.getOrCreateId(string.substr(I - 4));
      memcpy(data.data() + 4, string.data(), I - 4);
      *reinterpret_cast<uint32_t*>(data.data() + I) = idx;
   }
}

template <size_t I>
std::string String<I>::toString(const BidirectionalMap<std::string>& dictionary) const {
   const uint32_t length = *reinterpret_cast<const uint32_t*>(data.data());
   if (length <= I) {
      const char* payload = reinterpret_cast<const char*>(data.data() + 4);
      return {payload, length};
   }
   const char* prefix = reinterpret_cast<const char*>(data.data() + 4);
   const Idx idx = *reinterpret_cast<const uint32_t*>(data.data() + I);
   std::string result(prefix, I - 4);
   result += dictionary.getValue(idx);
   return result;
}

template <size_t I>
std::optional<common::String<I>> String<I>::embedString(
   const std::string& string,
   const BidirectionalMap<std::string>& dictionary
) {
   String result;
   const uint32_t length = string.length();
   *reinterpret_cast<uint32_t*>(result.data.data()) = length;
   if (length <= I) {
      memcpy(result.data.data() + 4, string.data(), length);
      memset(result.data.data() + 4 + length, '\0', I - length);
      return result;
   }
   auto idx = dictionary.getId(string.substr(I - 4));
   if (idx.has_value()) {
      memcpy(result.data.data() + 4, string.data(), I - 4);
      *reinterpret_cast<uint32_t*>(result.data.data() + I) = idx.value();
      return result;
   }
   return std::nullopt;
}

template <size_t I>
int String<I>::compare(const String<I>& other) const {
   return memcmp(this->data.data(), other.data.data(), I + 4);
}

template <size_t I>
bool String<I>::operator==(const String<I>& other) const {
   return this->compare(other) == 0;
}

template <size_t I>
bool String<I>::operator<(const String<I>& other) const {
   const int prefix_compare = memcmp(this->data.data() + 4, other.data.data() + 4, 8);
   if (prefix_compare < 0) {
      return true;
   }
   if (prefix_compare > 0) {
      return false;
   }
   // TODO(#137) implement more than just prefix matching
   return false;
}

template <size_t I>
bool String<I>::operator<=(const String<I>& other) const {
   const int prefix_compare = memcmp(this->data.data() + 4, other.data.data() + 4, 8);
   if (prefix_compare < 0) {
      return true;
   }
   if (prefix_compare > 0) {
      return false;
   }
   // TODO(#137) implement more than just prefix matching
   return true;
}

template <size_t I>
bool String<I>::operator>(const String<I>& other) const {
   return other < *this;
}

template <size_t I>
bool String<I>::operator>=(const String<I>& other) const {
   return other <= *this;
}

template <size_t I>
bool String<I>::operator!=(const String<I>& other) const {
   return !(*this == other);
}

template class String<STRING_SIZE>;

}  // namespace silo::common

template <size_t I>
std::size_t std::hash<silo::common::String<I>>::operator()(const silo::common::String<I>& str
) const {
   const std::string_view str_view(str.data.data(), str.data.size());
   return std::hash<std::string_view>{}(str_view);
}

template class std::hash<silo::common::String<silo::common::STRING_SIZE>>;
