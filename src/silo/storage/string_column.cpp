#include "silo/storage/string_column.h"

#include <utility>

#include "silo/storage/dictionary.h"

namespace silo::storage {

roaring::Roaring RawStringColumn::filter(std::string value) const {
   return silo::storage::RawBaseColumn<std::string>::filter(value);
}

IndexedStringColumn::IndexedStringColumn(
   std::string column_name,
   const silo::Dictionary& dictionary,
   std::vector<roaring::Roaring> indexed_values
)
    : column_name(std::move(column_name)),
      dictionary(dictionary),
      indexed_values(std::move(indexed_values)) {}

roaring::Roaring IndexedStringColumn::filter(std::string value) const {
   const auto value_id = dictionary.lookupValueId(column_name, value);
   if (value_id.has_value()) {
      return indexed_values[*value_id];
   }
   return {};
}

}  // namespace silo::storage
