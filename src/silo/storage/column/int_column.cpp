#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition() = default;

const std::vector<int32_t>& IntColumnPartition::getValues() const {
   return values;
}

void IntColumnPartition::insert(const std::string& value) {
   try {
      const int32_t int_value = value.empty() ? INT32_MIN : std::stoi(value);
      values.push_back(int_value);
   } catch (std::logic_error& err) {
      throw std::runtime_error("Wrong format for Integer: '" + value + "'");
   }
}

void IntColumnPartition::insertNull() {
   values.push_back(INT32_MIN);
}

IntColumn::IntColumn() = default;

IntColumnPartition& IntColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
