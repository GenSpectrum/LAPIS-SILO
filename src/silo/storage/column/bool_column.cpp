#include "silo/storage/column/bool_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

BoolColumnPartition::BoolColumnPartition(std::string column_name)
    : column_name(std::move(column_name)) {}

const std::vector<silo::common::OptionalBool>& BoolColumnPartition::getValues() const {
   return values;
}

void BoolColumnPartition::insert(bool value) {
   values.emplace_back(value);
}

void BoolColumnPartition::insertNull() {
   values.emplace_back();
}

void BoolColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

BoolColumn::BoolColumn(std::string column_name)
    : column_name(std::move(column_name)) {}

BoolColumnPartition& BoolColumn::createPartition() {
   return partitions.emplace_back(column_name);
}

}  // namespace silo::storage::column
