#pragma once

#include <string>

#include <fmt/format.h>

#include <boost/serialization/access.hpp>

namespace rhydb::common {

class LineageName {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & string;
      // clang-format on
   }

  public:
   std::string string;

   bool operator==(const LineageName& other) const;
};

}  // namespace rhydb::common

namespace std {
template <>
struct hash<rhydb::common::LineageName> {
   std::size_t operator()(const rhydb::common::LineageName& lineage_name) const;
};
}  // namespace std

template <>
struct [[maybe_unused]] fmt::formatter<rhydb::common::LineageName> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const rhydb::common::LineageName& lineage_name,
      format_context& ctx
   ) -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", lineage_name.string);
   }
};
