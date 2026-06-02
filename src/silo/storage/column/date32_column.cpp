#include "silo/storage/column/date32_column.h"

#include <expected>

#include "silo/common/date32.h"

namespace silo::storage::column {

Date32Column::Date32Column(ColumnMetadata* metadata)
    : metadata(metadata) {}

bool Date32Column::isSorted() const {
   return is_sorted;
}

std::expected<void, std::string> Date32Column::appendChunk(const Buffer& buffer) {
   values.reserve(values.size() + buffer.size());
   for (const auto& value : buffer) {
      if (value.has_value()) {
         if (!values.empty() && *value < values.back()) {
            is_sorted = false;
         }
         values.push_back(*value);
      } else {
         null_bitmap.add(values.size());
         // We need to insert _some_ value to keep vector size correct. However, it will never
         // be read
         values.push_back(0);
         is_sorted = false;
      }
   }
   return {};
}

const std::vector<silo::common::Date32>& Date32Column::getValues() const {
   return values;
}

}  // namespace silo::storage::column
