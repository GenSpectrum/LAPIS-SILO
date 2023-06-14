#include "silo/storage/column/int_column.h"

namespace silo::storage::column {

IntColumn::IntColumn() = default;

const std::vector<uint64_t>& IntColumn::getValues() const {
   return values;
}

void IntColumn::insert(uint64_t value) {
   values.push_back(value);
}

std::string IntColumn::getAsString(std::size_t idx) const {
   return std::to_string(values[idx]);
}

}  // namespace silo::storage::column
