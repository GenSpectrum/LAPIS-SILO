#include "silo/storage/date_column.h"

#include <algorithm>

namespace silo::storage {

RawDateColumn::RawDateColumn(
   std::string column_name,
   std::vector<std::chrono::year_month_day> values
)
    : column_name(std::move(column_name)),
      values(std::move(values)) {}

roaring::Roaring RawDateColumn::filterRange(
   const std::chrono::year_month_day& from_date,
   const std::chrono::year_month_day& to_date
) const {
   roaring::Roaring final_bitmap;

   for (size_t i = 0; i < values.size(); ++i) {
      const auto current_value_date = values[i];
      if (current_value_date >= from_date && current_value_date <= to_date) {
         final_bitmap.add(i);
      }
   }

   return final_bitmap;
}

SortedDateColumn::SortedDateColumn(
   std::string column_name,
   std::vector<std::chrono::year_month_day> values
)
    : column_name(std::move(column_name)),
      values(std::move(values)) {}

roaring::Roaring SortedDateColumn::filterRange(
   const std::chrono::year_month_day& from_date,
   const std::chrono::year_month_day& to_date
) const {
   auto lower = std::lower_bound(values.begin(), values.end(), from_date);
   auto upper = std::upper_bound(values.begin(), values.end(), to_date);

   const size_t lower_index = std::distance(values.begin(), lower);
   const size_t upper_index =
      std::min(std::distance(values.begin(), upper), static_cast<std::ptrdiff_t>(values.size()));

   roaring::Roaring result;
   result.addRange(lower_index, upper_index);
   return result;
}

}  // namespace silo::storage
