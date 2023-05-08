#include "silo/storage/string_column.h"

#include <utility>

#include "silo/storage/dictionary.h"

namespace silo::storage {

RawStringColumn::RawStringColumn(std::string columnName, std::vector<std::string> values)
    : columnName(std::move(columnName)),
      values(std::move(values)) {}

const roaring::Roaring RawStringColumn::filter(std::string value) const {
   roaring::Roaring filtered_bitmap;
   for (size_t i = 0; i < values.size(); ++i) {
      if (values[i] == value) {
         filtered_bitmap.add(i);
      }
   }
   return filtered_bitmap;
}

IndexedStringColumn::IndexedStringColumn(
   std::string column_name,
   const silo::Dictionary& dictionary,
   std::vector<roaring::Roaring> indexed_values
)
    : column_name(std::move(column_name)),
      dictionary(dictionary),
      indexed_values(std::move(indexed_values)) {}

const roaring::Roaring IndexedStringColumn::filter(std::string value) const {
   const auto value_id = dictionary.lookupValueId(column_name, value);
   if (value_id.has_value()) {
      return indexed_values[*value_id];
   }
   return roaring::Roaring();
}

}  // namespace silo::storage
