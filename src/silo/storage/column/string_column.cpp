#include "silo/storage/column/string_column.h"

namespace silo::storage::column {

RawStringColumn::RawStringColumn() = default;

void RawStringColumn::insert(const std::string& value) {
   values.push_back(value);
}

const std::vector<std::string>& RawStringColumn::getValues() const {
   return values;
}

IndexedStringColumn::IndexedStringColumn() = default;

roaring::Roaring IndexedStringColumn::filter(const std::string& value) const {
   if (!value_id_lookup.contains(value)) {
      return {};
   }

   return indexed_values[value_id_lookup.at(value)];
}

void IndexedStringColumn::insert(const std::string& value) {
   if (!value_id_lookup.contains(value)) {
      value_id_lookup[value] = value_id_lookup.size();
      indexed_values.emplace_back();
   }

   const auto value_id = value_id_lookup[value];
   indexed_values[value_id].add(sequence_count);
   sequence_count++;
}

}  // namespace silo::storage::column
