#include "silo/common/lineage_name.h"

#include <cstddef>

namespace std {
using silo::common::LineageName;
std::size_t std::hash<LineageName>::operator()(const LineageName& lineage_name) const {
   return std::hash<std::string>()(lineage_name.string);
}
}  // namespace std

namespace silo::common {

bool LineageName::operator==(const LineageName& other) const {
   return string == other.string;
}

}  // namespace silo::common