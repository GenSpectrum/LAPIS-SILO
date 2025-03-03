#include "silo/storage/column/string_column.h"

#include <string>

#include "silo/common/bidirectional_map.h"
#include "silo/common/string.h"

using silo::common::String;
using silo::common::STRING_SIZE;

namespace silo::storage::column {

StringColumnPartition::StringColumnPartition(
   std::string column_name,
   silo::common::BidirectionalMap<std::string>* lookup
)
    : column_name(std::move(column_name)),
      lookup(lookup) {}

void StringColumnPartition::insert(const std::string& value) {
   const String<STRING_SIZE> tmp(value, *lookup);
   values.push_back(tmp);
}

void StringColumnPartition::insertNull() {
   const String<STRING_SIZE> tmp("", *lookup);
   values.push_back(tmp);
}

void StringColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

const std::vector<String<STRING_SIZE>>& StringColumnPartition::getValues() const {
   return values;
}

std::optional<String<STRING_SIZE>> StringColumnPartition::embedString(const std::string& string
) const {
   return String<STRING_SIZE>::embedString(string, *lookup);
}

StringColumn::StringColumn(std::string column_name)
    : column_name(std::move(column_name)) {}

StringColumnPartition& StringColumn::createPartition() {
   return partitions.emplace_back(column_name, &lookup);
}

std::optional<String<STRING_SIZE>> StringColumn::embedString(const std::string& string) const {
   return String<STRING_SIZE>::embedString(string, lookup);
}

}  // namespace silo::storage::column
