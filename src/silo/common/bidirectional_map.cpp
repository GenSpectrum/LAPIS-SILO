#include "silo/common/bidirectional_map.h"

#include <optional>

#include "silo/common/pango_lineage.h"

namespace silo::common {

template <typename V>
BidirectionalMap<V>::BidirectionalMap()
    : id_to_value(),
      value_to_id() {}

template <typename V>
V BidirectionalMap<V>::getValue(Idx idx) const {
   return id_to_value.at(idx);
}

template <typename V>
std::optional<Idx> BidirectionalMap<V>::getId(V value) const {
   if (value_to_id.contains(value)) {
      return value_to_id.at(value);
   }
   return std::nullopt;
}

template <typename V>
Idx BidirectionalMap<V>::getOrCreateId(V value) {
   if (value_to_id.contains(value)) {
      return value_to_id.at(value);
   }
   const Idx identifier = id_to_value.size();
   id_to_value.push_back(value);
   value_to_id[value] = identifier;
   return identifier;
}

template class BidirectionalMap<PangoLineage>;
template class BidirectionalMap<std::string>;

}  // namespace silo::common
