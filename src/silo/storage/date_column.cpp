#include "silo/storage/date_column.h"

#include <algorithm>

namespace silo::storage {

using std::chrono::days;

roaring::Roaring RawDateColumn::filterRange(
   const std::chrono::year_month_day& from_date,
   const std::chrono::year_month_day& to_date
) const {
   const std::chrono::sys_days from_sys_days = std::chrono::sys_days{from_date};
   const std::chrono::sys_days to_sys_days = std::chrono::sys_days{to_date};

   roaring::Roaring final_bitmap;
   for (auto current_date = from_sys_days; current_date <= to_sys_days; current_date += days(1)) {
      const roaring::Roaring current_bitmap = RawBaseColumn::filter(current_date);
      final_bitmap |= current_bitmap;
   }

   return final_bitmap;
}

SortedDateColumn::SortedDateColumn(std::string column_name, std::vector<std::chrono::year_month_day> values)
    : column_name(std::move(column_name)),
      values(std::move(values)) {}

roaring::Roaring SortedDateColumn::filterRange(
   const std::chrono::year_month_day& from_date,
   const std::chrono::year_month_day& to_date
) const {
   auto lower = std::lower_bound(values.begin(), values.end(), from_date);
   auto upper = std::upper_bound(values.begin(), values.end(), to_date);

   const size_t lower_index = std::distance(values.begin(), lower);
   const size_t upper_index = std::min(
      std::distance(values.begin(), upper), static_cast<std::ptrdiff_t>(values.size())
   );

   roaring::Roaring result;
   result.addRange(lower_index, upper_index);
   return result;
}

}  // namespace silo::storage
