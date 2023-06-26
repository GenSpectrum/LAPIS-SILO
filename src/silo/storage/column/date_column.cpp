#include "silo/storage/column/date_column.h"

#include "silo/common/date.h"

namespace silo::storage::column {

DateColumnPartition::DateColumnPartition(bool is_sorted)
    : is_sorted(is_sorted) {}

bool DateColumnPartition::isSorted() const {
   return is_sorted;
}

void DateColumnPartition::insert(const silo::common::Date& value) {
   values.push_back(value);
}

const std::vector<silo::common::Date>& DateColumnPartition::getValues() const {
   return values;
}

DateColumn::DateColumn()
    : DateColumn::DateColumn(false) {}

DateColumn::DateColumn(bool is_sorted)
    : is_sorted(is_sorted) {}

DateColumnPartition DateColumn::createPartition() {
   return DateColumnPartition(is_sorted);
}

}  // namespace silo::storage::column
