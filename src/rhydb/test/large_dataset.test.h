#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <nlohmann/json.hpp>

namespace rhydb::test {

/// Helper method to build large datasets that span more than one chunk.
/// `make_row(row_index)` returns a complete, schema-valid row for the given global row index.
inline std::vector<nlohmann::json> buildLargeTestDataset(
   const std::function<nlohmann::json(size_t row_index)>& make_row,
   const size_t target_size
) {
   std::vector<nlohmann::json> rows;
   rows.reserve(target_size);
   for (size_t row_index = 0; row_index < target_size; ++row_index) {
      rows.push_back(make_row(row_index));
   }
   return rows;
}

}  // namespace rhydb::test
