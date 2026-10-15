#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "rhydb/common/panic.h"
#include "rhydb/roaring_util/roaring_container.h"
#include "rhydb/storage/column/column.h"

namespace rhydb::storage::column {

/// Assigns values to the rows of several chunks of `column`: `values[i]` holds, in ascending row
/// order, the values of the rows `rows[i]` selects within its chunk (see the column's per-chunk
/// `update`).
template <Column ColumnType>
void updateChunks(
   ColumnType& column,
   const std::vector<std::pair<uint16_t, roaring_util::RoaringContainerView>>& rows,
   const std::vector<typename ColumnType::Buffer>& values
) {
   RHYDB_ASSERT_EQ(rows.size(), values.size());
   for (size_t chunk = 0; chunk < rows.size(); ++chunk) {
      const auto& [chunk_id, chunk_rows] = rows[chunk];
      column.update(chunk_id, chunk_rows, values[chunk]);
   }
}

}  // namespace rhydb::storage::column
