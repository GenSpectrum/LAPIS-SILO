#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <optional>
#include <string>

#include <boost/serialization/access.hpp>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/panic.h"
#include "silo/storage/vector/variable_data_registry.h"

namespace silo {

// Umbra strings as described in https://www.cidrdb.org/cidr2020/papers/p29-neumann-cidr20.pdf
// aka GermanString as popularized by Andy Pavlo
// But with a templated size
template <size_t I, typename suffix_id_type>
class GermanString {
   // No space left to save prefix if suffix_id_type is too large
   static_assert(I > sizeof(suffix_id_type));

   friend class boost::serialization::access;
   friend class std::hash<silo::GermanString<I, suffix_id_type>>;

  public:
   using length_type = uint32_t;
   // Strings up until this size are stored in-place
   constexpr static size_t SHORT_STRING_SIZE = I;
   constexpr static size_t PREFIX_START = sizeof(length_type);
   constexpr static size_t PREFIX_LENGTH = I - sizeof(suffix_id_type);
   constexpr static size_t SUFFIX_ID_START = PREFIX_START + PREFIX_LENGTH;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & data;
      // clang-format on
   }

  private:
   std::array<std::byte, I + sizeof(length_type)> data;

   GermanString() = default;

  public:
   GermanString(length_type length, std::string_view prefix, suffix_id_type suffix_offset) {
      SILO_ASSERT(length > SHORT_STRING_SIZE);
      SILO_ASSERT(prefix.size() == PREFIX_LENGTH);
      SILO_ASSERT(PREFIX_START + PREFIX_LENGTH <= data.size());
      SILO_ASSERT(SUFFIX_ID_START + sizeof(suffix_id_type) <= data.size());
      *reinterpret_cast<length_type*>(data.data()) = length;
      std::memcpy(data.data() + PREFIX_START, prefix.data(), PREFIX_LENGTH);
      *reinterpret_cast<suffix_id_type*>(data.data() + SUFFIX_ID_START) = suffix_offset;
   }

   explicit GermanString(std::string_view short_string) {
      length_type short_string_length = short_string.size();
      SILO_ASSERT(short_string_length <= SHORT_STRING_SIZE);
      *reinterpret_cast<length_type*>(data.data()) = short_string_length;
      std::memcpy(data.data() + PREFIX_START, short_string.data(), short_string_length);
   }

   std::string dataAsHexString() const {
      std::stringstream sstream;
      sstream << "0x" << std::setfill('0') << std::setw(static_cast<int>(data.size() * 2))
              << std::hex << data.data();
      return sstream.str();
   }

   inline length_type length() const { return *reinterpret_cast<const length_type*>(data.data()); }

   inline bool isInPlace() const { return length() <= I; }

   std::string_view getShortString() const {
      return std::string_view{reinterpret_cast<const char*>(data.data() + PREFIX_START), length()};
   }

   std::string_view prefix() const {
      return std::string_view{
         reinterpret_cast<const char*>(data.data() + PREFIX_START), PREFIX_LENGTH
      };
   }

   suffix_id_type suffixId() const {
      return *reinterpret_cast<const suffix_id_type*>(data.data() + SUFFIX_ID_START);
   }

   // Compares this to other without looking up the suffix. We might be able to differentiate using
   // only the prefix and length
   std::optional<std::strong_ordering> fastCompare(std::string_view other) const {
      if (length() <= SHORT_STRING_SIZE) {
         auto this_string = this->getShortString();
         return std::lexicographical_compare_three_way(
            this_string.begin(), this_string.end(), other.begin(), other.end()
         );
      }

      const int prefix_compare = std::memcmp(this->prefix().data(), other.data(), PREFIX_LENGTH);
      if (prefix_compare < 0) {
         return std::strong_ordering::less;
      }
      if (prefix_compare > 0) {
         return std::strong_ordering::greater;
      }
      // Prefix matches, so we cannot decide without looking at the suffix
      return std::nullopt;
   }

   bool operator==(const GermanString<I, suffix_id_type>& other) const {
      return memcmp(this->data.data(), other.data.data(), I + sizeof(length_type)) == 0;
   }

   bool operator!=(const GermanString<I, suffix_id_type>& other) const { return !(*this == other); }
};

using SiloString = GermanString<12, storage::vector::VariableDataRegistry::Identifier>;

}  // namespace silo

template <size_t I, typename suffix_id_type>
struct std::hash<silo::GermanString<I, suffix_id_type>> {
   std::size_t operator()(const silo::GermanString<I, suffix_id_type>& str) const {
      const std::string_view str_view(str.data.data(), str.data.size());
      return std::hash<std::string_view>{}(str_view);
   }
};
