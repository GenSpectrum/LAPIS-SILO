#pragma once

#include <cstdint>

#include "silo/storage/column/numeric_column.h"

namespace silo::storage::column {

using Int64Column = NumericColumn<int64_t>;
using Int64ColumnBuilder = NumericColumnBuilder<int64_t>;

}  // namespace silo::storage::column
