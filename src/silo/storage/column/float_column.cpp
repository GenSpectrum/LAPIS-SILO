#include "silo/storage/column/float_column.h"

#include <cmath>
#include <stdexcept>

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition() = default;

const std::vector<double>& FloatColumnPartition::getValues() const {
   return values;
}

void FloatColumnPartition::insert(const std::string& value) {
   double double_value;
   try {
      double_value = value.empty() ? std::nan("") : std::stod(value);
   } catch (std::logic_error& err) {
      throw std::runtime_error("Bad format for double value: '" + value + "'");
   }
   values.push_back(double_value);
}

void FloatColumnPartition::insertNull() {
   values.push_back(std::nan(""));
}

void FloatColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

FloatColumn::FloatColumn() = default;

FloatColumnPartition& FloatColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
