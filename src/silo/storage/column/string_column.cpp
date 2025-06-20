#include "silo/storage/column/string_column.h"

#include <string>

#include "silo/common/bidirectional_map.h"
#include "silo/common/string.h"

using silo::common::String;
using silo::common::STRING_SIZE;

namespace silo::storage::column {

std::optional<String<STRING_SIZE>> StringColumnMetadata::embedString(const std::string& string
) const {
   return String<STRING_SIZE>::embedString(string, dictionary);
}

StringColumnPartition::StringColumnPartition(StringColumnMetadata* metadata)
    : metadata(metadata) {}

void StringColumnPartition::insert(const std::string& value) {
   const String<STRING_SIZE> tmp(value, metadata->dictionary);
   values.push_back(tmp);
}

void StringColumnPartition::insertNull() {
   const String<STRING_SIZE> tmp("", metadata->dictionary);
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
   return String<STRING_SIZE>::embedString(string, metadata->dictionary);
}

}  // namespace silo::storage::column
