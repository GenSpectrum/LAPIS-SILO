#include "silo/storage/date_column.h"

namespace silo::storage {

using std::chrono::days;

roaring::Roaring DateColumn::filterRange(
   const std::chrono::year_month_day& from_date,
   const std::chrono::year_month_day& to_date
) const {
   const std::chrono::sys_days from_sys_days = std::chrono::sys_days{from_date};
   const std::chrono::sys_days to_sys_days = std::chrono::sys_days{to_date};

   roaring::Roaring final_bitmap;
   for (auto current_date = from_sys_days; current_date <= to_sys_days; current_date += days(1)) {
      const roaring::Roaring current_bitmap = filter(current_date);
      final_bitmap |= current_bitmap;
   }

   return final_bitmap;
}

}  // namespace silo::storage
