#include "silo/storage/column/date_column.h"

namespace silo::storage::column {

DateColumn::DateColumn()
    : is_sorted(false) {}

DateColumn::DateColumn(bool is_sorted)
    : is_sorted(is_sorted) {}

bool DateColumn::isSorted() const {
   return is_sorted;
}

void DateColumn::insert(const std::time_t& value) {
   values.push_back(value);
}

const std::vector<std::time_t>& DateColumn::getValues() const {
   return values;
}

}  // namespace silo::storage::column
