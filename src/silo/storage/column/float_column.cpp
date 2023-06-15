#include "silo/storage/column/float_column.h"

namespace silo::storage::column {

FloatColumn::FloatColumn() = default;

const std::vector<double>& FloatColumn::getValues() const {
   return values;
}

void FloatColumn::insert(double value) {
   values.push_back(value);
}

std::string FloatColumn::getAsString(std::size_t idx) const {
   return std::to_string(values[idx]);
}

}  // namespace silo::storage::column
