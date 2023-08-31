#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition() = default;

const std::vector<int32_t>& IntColumnPartition::getValues() const {
   return values;
}

void IntColumnPartition::insert(const std::string& value) {
   int32_t int_value;
   try {
      int_value = value.empty() ? INT32_MIN : std::stoi(value);
   } catch (std::logic_error& err) {
      SPDLOG_INFO("Integer wrongly formatted: '" + value + "'\nInterpreting value as null");
      int_value = INT32_MIN;
   }
   values.push_back(int_value);
}

IntColumn::IntColumn() = default;

IntColumnPartition& IntColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
