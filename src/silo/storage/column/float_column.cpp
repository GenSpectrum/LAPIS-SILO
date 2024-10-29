#include "silo/storage/column/float_column.h"

#include <cmath>
#include <stdexcept>

#include <fmt/format.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition(std::string column_name)
    : column_name(std::move(column_name)) {}

const std::vector<double>& FloatColumnPartition::getValues() const {
   return values;
}

void FloatColumnPartition::insert(const std::string& value) {
   double double_value;
   try {
      double_value = value.empty() ? FloatColumn::null() : std::stod(value);
   } catch (std::logic_error& err) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Bad format for double value: '{}' in column '{}'", value, column_name)
      );
   }
   values.push_back(double_value);
}

void FloatColumnPartition::insertNull() {
   values.push_back(std::nan(""));
}

void FloatColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

FloatColumn::FloatColumn(std::string column_name)
    : column_name(std::move(column_name)) {}

FloatColumnPartition& FloatColumn::createPartition() {
   return partitions.emplace_back(column_name);
}

}  // namespace silo::storage::column
