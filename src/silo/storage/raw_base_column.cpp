#include "silo/storage/raw_base_column.h"

namespace silo::storage {

template <typename T>
RawBaseColumn<T>::RawBaseColumn(std::string column_name, std::vector<T> values)
    : column_name(std::move(column_name)),
      values(values) {}

template <typename T>
roaring::Roaring RawBaseColumn<T>::filter(const T& value) const {
   roaring::Roaring filtered_bitmap;
   for (size_t i = 0; i < values.size(); ++i) {
      if (values[i] == value) {
         filtered_bitmap.add(i);
      }
   }
   return filtered_bitmap;
}

template class RawBaseColumn<std::string>;
template class RawBaseColumn<std::chrono::year_month_day>;

}  // namespace silo::storage
