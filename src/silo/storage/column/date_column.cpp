#include "silo/storage/column/date_column.h"

#include "silo/common/date.h"

namespace silo::storage::column {

DateColumn::DateColumn()
    : is_sorted(false) {}

DateColumn::DateColumn(bool is_sorted)
    : is_sorted(is_sorted) {}

bool DateColumn::isSorted() const {
   return is_sorted;
}

void DateColumn::insert(const silo::common::Date& value) {
   values.push_back(value);
}

const std::vector<silo::common::Date>& DateColumn::getValues() const {
   return values;
}

std::string DateColumn::getAsString(std::size_t idx) const {
   return silo::common::dateToString(values[idx]);
};

}  // namespace silo::storage::column
