#ifndef SILO_STRING_H
#define SILO_STRING_H

#include <optional>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>

#include "silo/common/bidirectional_map.h"
#include "silo/common/types.h"

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

   int compare(const String& other) const;

   String() = default;

  public:
   String(const std::string& string, BidirectionalMap<std::string>& dictionary);

   static std::optional<common::String<I>> embedString(
      const std::string& string,
      const BidirectionalMap<std::string>& dictionary
   );

   std::string toString(const BidirectionalMap<std::string>& dictionary) const;

   bool operator==(const String& other) const;

   bool operator<(const String& other) const;

   bool operator<=(const String& other) const;

   bool operator>(const String& other) const;

   bool operator>=(const String& other) const;

   bool operator!=(const String& other) const;
};

}  // namespace silo::common

template <size_t I>
struct std::hash<silo::common::String<I>> {
   std::size_t operator()(const silo::common::String<I>& str) const;
};

#endif  // SILO_STRING_H
