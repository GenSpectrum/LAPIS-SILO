#include "silo/storage/column/int_column.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

IntColumn::IntColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> IntColumn::insert(int32_t value) {
   values.push_back(value);
   return {};
}

void IntColumn::insertNull() {
   null_bitmap.add(values.size());
   values.push_back(0);
}

}  // namespace silo::storage::column
