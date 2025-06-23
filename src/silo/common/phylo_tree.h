#pragma once

#include <filesystem>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/common/tree_node_id.h"

namespace silo::common {
using silo::common::TreeNodeId;

class TreeNode {
  public:
   TreeNodeId node_id;
   std::vector<TreeNodeId> children;
   std::optional<TreeNodeId> parent;
   int depth;

   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
    archive & node_id;
    archive & children;
    archive & parent;
    archive & depth;
      // clang-format on
   }
};

class PhyloTree {
  public:
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>> nodes;

   static PhyloTree fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTree fromAuspiceJSONString(const std::string& json_string);

   static PhyloTree fromNewickFile(const std::filesystem::path& newick_path);

   static PhyloTree fromNewickString(const std::string& newick_string);

   static PhyloTree fromFile(const std::filesystem::path& path);

   private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const;

    template <class Archive>
    void load(Archive& ar, const unsigned int version);

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        boost::serialization::split_member(ar, *this, version);
   }
};

}  // namespace silo::common