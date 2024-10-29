#include "silo/storage/column/date_column.h"

#include "silo/common/date.h"

namespace silo::storage::column {

DateColumnPartition::DateColumnPartition(std::string column_name, bool is_sorted)
    : column_name(std::move(column_name)),
      is_sorted(is_sorted) {}

bool DateColumnPartition::isSorted() const {
   return is_sorted;
}

void DateColumnPartition::insert(const silo::common::Date& value) {
   values.push_back(value);
}

void DateColumnPartition::insertNull() {
   values.push_back(common::NULL_DATE);
}

void DateColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

const std::vector<silo::common::Date>& DateColumnPartition::getValues() const {
   return values;
}

DateColumn::DateColumn(std::string column_name, bool is_sorted)
    : column_name(std::move(column_name)),
      is_sorted(is_sorted) {}

DateColumnPartition& DateColumn::createPartition() {
   return partitions.emplace_back(column_name, is_sorted);
}

}  // namespace silo::storage::column
