#include "silo/storage/column/date_column.h"

#include "silo/common/date.h"

namespace silo::storage::column {

DateColumnPartition::DateColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata),
      is_sorted(true) {}

bool DateColumnPartition::isSorted() const {
   return is_sorted;
}

void DateColumnPartition::insert(std::string_view value) {
   const auto date_value = silo::common::stringToDate(value);
   if (!values.empty() && date_value < values.back()) {
      is_sorted = false;
   }
   values.push_back(date_value);
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

}  // namespace silo::storage::column
