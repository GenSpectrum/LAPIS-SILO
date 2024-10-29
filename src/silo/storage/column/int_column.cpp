#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition(std::string column_name)
    : column_name(std::move(column_name)) {}

const std::vector<int32_t>& IntColumnPartition::getValues() const {
   return values;
}

void IntColumnPartition::insert(const std::string& value) {
   try {
      const int32_t int_value = value.empty() ? IntColumn::null() : std::stoi(value);
      values.push_back(int_value);
   } catch (std::logic_error& err) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Wrong format for Integer: '{}' in column '{}'", value, column_name)
      );
   }
}

void IntColumnPartition::insertNull() {
   values.push_back(IntColumn::null());
}

void IntColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

IntColumn::IntColumn(std::string column_name)
    : column_name(std::move(column_name)) {}

IntColumnPartition& IntColumn::createPartition() {
   return partitions.emplace_back(column_name);
}

}  // namespace silo::storage::column
