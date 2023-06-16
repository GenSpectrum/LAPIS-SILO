#include "silo/storage/column/string_column.h"

namespace silo::storage::column {

RawStringColumn::RawStringColumn() = default;

void RawStringColumn::insert(const std::string& value) {
   values.push_back(value);
}

const std::vector<std::string>& RawStringColumn::getValues() const {
   return values;
}

std::string RawStringColumn::getAsString(std::size_t idx) const {
   return values[idx];
};

IndexedStringColumn::IndexedStringColumn() = default;

roaring::Roaring IndexedStringColumn::filter(const std::string& value) const {
   if (!value_to_id_lookup.contains(value)) {
      return {};
   }

   return indexed_values[value_to_id_lookup.at(value)];
}

void IndexedStringColumn::insert(const std::string& value) {
   uint32_t value_id;
   if (!value_to_id_lookup.contains(value)) {
      value_id = value_to_id_lookup.size();
      value_to_id_lookup[value] = value_id;
      id_to_value_lookup.push_back(value);
      indexed_values.emplace_back();
   } else {
      value_id = value_to_id_lookup[value];
   }

   indexed_values[value_id].add(value_ids.size());
   value_ids.push_back(value_id);
}

std::string IndexedStringColumn::getAsString(std::size_t idx) const {
   return id_to_value_lookup[value_ids[idx]];
};

}  // namespace silo::storage::column
