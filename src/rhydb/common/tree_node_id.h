#pragma once

#include <string>

#include <fmt/format.h>

#include <boost/serialization/access.hpp>

namespace rhydb::common {

class TreeNodeId {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & string;
      // clang-format on
   }

  public:
   std::string string;

   bool operator==(const TreeNodeId& other) const;
   bool operator<(const TreeNodeId& other) const;
};

}  // namespace rhydb::common

namespace std {
template <>
struct hash<rhydb::common::TreeNodeId> {
   std::size_t operator()(const rhydb::common::TreeNodeId& tree_node_id) const;
};
}  // namespace std

template <>
struct [[maybe_unused]] fmt::formatter<rhydb::common::TreeNodeId> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const rhydb::common::TreeNodeId& tree_node_id,
      format_context& ctx
   ) -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", tree_node_id.string);
   }
};
