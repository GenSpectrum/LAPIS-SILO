#include "silo/common/tree_node_id.h"

#include <cstddef>

namespace std {
using silo::common::TreeNodeId;
std::size_t std::hash<TreeNodeId>::operator()(const TreeNodeId& tree_node_id) const {
   return std::hash<std::string>()(tree_node_id.string);
}
}  // namespace std

namespace silo::common {

bool TreeNodeId::operator==(const TreeNodeId& other) const {
   return string == other.string;
}

}  // namespace silo::common