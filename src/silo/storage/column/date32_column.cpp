#include "silo/storage/column/date32_column.h"

#include <expected>

#include "silo/common/date32.h"

namespace silo::storage::column {

Date32Column::Date32Column(ColumnMetadata* metadata)
    : metadata(metadata) {}

bool Date32Column::isSorted() const {
   return is_sorted;
}

std::expected<void, std::string> Date32Column::insert(std::string_view value) {
   auto date_result = silo::common::stringToDate32(value);
   if (!date_result.has_value()) {
      return std::unexpected{date_result.error()};
   }
   const auto date_value = date_result.value();
   if (!values.empty() && date_value < values.back()) {
      is_sorted = false;
   }
   values.push_back(date_value);
   return {};
}

void Date32Column::insertNull() {
   const size_t row_id = values.size();
   null_bitmap.add(row_id);
   // We need to insert _some_ value to keep vector size correct. However, it will never be read
   values.push_back(0);
   is_sorted = false;
}

void Date32Column::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

const std::vector<silo::common::Date32>& Date32Column::getValues() const {
   return values;
}

}  // namespace silo::storage::column
