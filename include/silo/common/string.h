#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace boost::serialization {
class access;
}  // namespace boost::serialization
namespace silo::common {
template <typename V>
class BidirectionalMap;
}  // namespace silo::common

namespace silo::common {

constexpr size_t STRING_SIZE = 16;

// Umbra strings as described in https://www.cidrdb.org/cidr2020/papers/p29-neumann-cidr20.pdf
// But with a templatized size
template <size_t I>
struct String {
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive& data;
      // clang-format on
   }

   std::array<char, I + 4> data;

   std::optional<std::strong_ordering> fastCompare(const String& other) const;

   String() = default;

  public:
   String(const std::string& string, BidirectionalMap<std::string>& dictionary);

   static std::optional<common::String<I>> embedString(
      const std::string& string,
      const BidirectionalMap<std::string>& dictionary
   );

   [[nodiscard]] std::string toString(const BidirectionalMap<std::string>& dictionary) const;

   bool operator==(const String& other) const;

   bool operator!=(const String& other) const;
};

typedef common::String<STRING_SIZE> SiloString;

}  // namespace silo::common

template <size_t I>
struct std::hash<silo::common::String<I>> {
   std::size_t operator()(const silo::common::String<I>& str) const;
};
