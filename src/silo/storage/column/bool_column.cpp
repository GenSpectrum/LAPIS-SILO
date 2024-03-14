#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

BoolColumnPartition::BoolColumnPartition() = default;

const std::vector<bool32_t>& BoolColumnPartition::getValues() const {
   return values;
}

void BoolColumnPartition::insert(const std::string& value) {
   try {
      const bool32_t bool_value = value.empty() ? BOOL32_MIN : std::stoi(value);
      values.push_back(bool_value);
   } catch (std::logic_error& err) {
      throw std::runtime_error("Wrong format for Booleger: '" + value + "'");
   }
}

void BoolColumnPartition::insertNull() {
   values.push_back(BOOL32_MIN);
}

void BoolColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

BoolColumn::BoolColumn() = default;

BoolColumnPartition& BoolColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
