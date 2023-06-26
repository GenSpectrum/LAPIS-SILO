#include "silo/storage/column/string_column.h"

#include <string>

#include "silo/common/bidirectional_map.h"

using silo::common::String;
using silo::common::STRING_SIZE;

namespace silo::storage::column {

StringColumnPartition::StringColumnPartition(silo::common::BidirectionalMap<std::string>& lookup)
    : lookup(lookup) {}

void StringColumnPartition::insert(const std::string& value) {
   values.push_back(String<STRING_SIZE>(value, lookup));
}

const std::vector<String<STRING_SIZE>>& StringColumnPartition::getValues() const {
   return values;
}

std::optional<String<STRING_SIZE>> StringColumnPartition::embedString(std::string string) const {
   return String<STRING_SIZE>::embedString(string, lookup);
}

StringColumn::StringColumn() {
   lookup = std::make_unique<common::BidirectionalMap<std::string>>();
};

StringColumnPartition StringColumn::createPartition() {
   return StringColumnPartition(*lookup);
}

std::optional<String<STRING_SIZE>> StringColumn::embedString(std::string string) const {
   return String<STRING_SIZE>::embedString(string, *lookup);
}

}  // namespace silo::storage::column
