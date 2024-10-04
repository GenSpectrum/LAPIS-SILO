#pragma once

#include <string>

#include <fmt/format.h>

#include <boost/serialization/access.hpp>

namespace silo::common {

class LineageName {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & string;
      // clang-format on
   }

  public:
   std::string string;

   bool operator==(const LineageName& other) const;
};

}  // namespace silo::common

namespace std {
template <>
struct hash<silo::common::LineageName> {
   std::size_t operator()(const silo::common::LineageName& lineage_name) const;
};
}  // namespace std

template <>
struct [[maybe_unused]] fmt::formatter<silo::common::LineageName> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::common::LineageName& lineage_name,
      format_context& ctx
   ) -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", lineage_name.string);
   }
};
