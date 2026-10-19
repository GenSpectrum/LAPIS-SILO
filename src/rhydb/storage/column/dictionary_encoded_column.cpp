#include "rhydb/storage/column/dictionary_encoded_column.h"

#include <algorithm>
#include <compare>
#include <optional>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include "rhydb/common/german_string.h"
#include "rhydb/common/panic.h"
#include "rhydb/storage/column/row_id.h"

namespace rhydb::storage::column {

size_t DictionaryEncodedColumnMetadata::numValues() const {
   size_t num_values = 0;
   for (size_t chunk_id = 0; chunk_id < values.numChunks(); ++chunk_id) {
      num_values += values.chunkSize(chunk_id);
   }
   return num_values;
}

std::string DictionaryEncodedColumnMetadata::getValue(Idx value_id) const {
   return values.getValueString(RowId::fromGlobal(value_id));
}

std::optional<Idx> DictionaryEncodedColumnMetadata::getId(std::string_view value) const {
   // `insertValues` fills every chunk but the last one completely, so the ids are contiguous
   Idx low = 0;
   auto high = static_cast<Idx>(numValues());
   while (low < high) {
      const Idx mid = low + ((high - low) / 2);
      const RowId row_id = RowId::fromGlobal(mid);
      auto ordering = values.getValue(row_id).fastCompare(value);
      if (!ordering.has_value()) {
         ordering = compareBytesUnsigned(values.getValueString(row_id), value);
      }
      if (*ordering == std::strong_ordering::equal) {
         return mid;
      }
      if (*ordering == std::strong_ordering::less) {
         low = mid + 1;
      } else {
         high = mid;
      }
   }
   return std::nullopt;
}

std::vector<Idx> DictionaryEncodedColumnMetadata::insertValues(
   const std::vector<std::string_view>& new_values
) {
   const auto old_num_values = static_cast<Idx>(numValues());
   std::vector<Idx> old_to_new_id;
   old_to_new_id.reserve(old_num_values);

   StringColumn merged{&values_metadata};
   StringColumn::Buffer chunk;
   chunk.reserve(COLUMN_CHUNK_SIZE);
   const auto flush = [&] {
      const auto appended = merged.appendChunk(chunk);
      RHYDB_ASSERT(appended.has_value());
      chunk.clear();
   };
   const auto push = [&](std::string value) {
      chunk.emplace_back(std::move(value));
      if (chunk.size() == COLUMN_CHUNK_SIZE) {
         flush();
      }
   };

   Idx next_id = 0;
   auto new_value = new_values.begin();
   for (Idx old_id = 0; old_id < old_num_values; ++old_id) {
      std::string old_value = getValue(old_id);
      for (; new_value != new_values.end() && compareBytesUnsigned(*new_value, old_value) < 0;
           ++new_value) {
         push(std::string{*new_value});
         ++next_id;
      }
      RHYDB_ASSERT(new_value == new_values.end() || *new_value != old_value);
      push(std::move(old_value));
      old_to_new_id.push_back(next_id++);
   }
   for (; new_value != new_values.end(); ++new_value) {
      push(std::string{*new_value});
   }
   if (!chunk.empty()) {
      flush();
   }

   values = std::move(merged);
   return old_to_new_id;
}

DictionaryEncodedColumn::DictionaryEncodedColumn(DictionaryEncodedColumnMetadata* metadata)
    : metadata(metadata) {}

std::optional<const roaring::Roaring*> DictionaryEncodedColumn::filter(Idx value_id) const {
   if (indexed_values.contains(value_id)) {
      return &indexed_values.at(value_id);
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> DictionaryEncodedColumn::filter(
   const std::optional<std::string>& value
) const {
   if (value == std::nullopt) {
      return &null_bitmap;
   }
   const auto& value_id = metadata->getId(value.value());
   if (!value_id.has_value()) {
      return std::nullopt;
   }
   return filter(value_id.value());
}

std::unordered_map<std::string_view, Idx> DictionaryEncodedColumn::internValues(
   const Buffer& buffer
) {
   std::vector<std::string_view> distinct_values;
   distinct_values.reserve(buffer.size());
   for (const auto& value : buffer) {
      distinct_values.emplace_back(value.has_value() ? std::string_view{*value} : "");
   }
   std::ranges::sort(distinct_values);
   const auto [first_duplicate, end] = std::ranges::unique(distinct_values);
   distinct_values.erase(first_duplicate, end);

   std::unordered_map<std::string_view, Idx> value_ids_by_value;
   std::vector<std::string_view> missing_values;
   for (const std::string_view value : distinct_values) {
      if (auto value_id = metadata->getId(value)) {
         value_ids_by_value.emplace(value, *value_id);
      } else {
         missing_values.push_back(value);
      }
   }
   if (missing_values.empty()) {
      return value_ids_by_value;
   }

   const std::vector<Idx> old_to_new_id = metadata->insertValues(missing_values);
   for (size_t chunk_id = 0; chunk_id < value_ids.numChunks(); ++chunk_id) {
      for (uint32_t row_in_chunk = 0; row_in_chunk < value_ids.chunkSize(chunk_id);
           ++row_in_chunk) {
         const RowId row_id{
            .chunk_id = static_cast<uint16_t>(chunk_id),
            .row_in_chunk = static_cast<uint16_t>(row_in_chunk),
         };
         value_ids.setValue(row_id, old_to_new_id.at(value_ids.at(row_id)));
      }
   }
   std::unordered_map<Idx, roaring::Roaring> remapped_indexed_values;
   for (auto& [old_id, bitmap] : indexed_values) {
      remapped_indexed_values.emplace(old_to_new_id.at(old_id), std::move(bitmap));
   }
   indexed_values = std::move(remapped_indexed_values);

   // The ids of the values that were already present have shifted as well, so look all up anew
   value_ids_by_value.clear();
   for (const std::string_view value : distinct_values) {
      value_ids_by_value.emplace(value, metadata->getId(value).value());
   }
   return value_ids_by_value;
}

std::expected<void, std::string> DictionaryEncodedColumn::appendChunk(const Buffer& buffer) {
   const auto ids_by_value = internValues(buffer);
   const uint32_t base = RowId::chunkStart(static_cast<uint16_t>(value_ids.numChunks()));
   std::vector<Idx> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      const uint32_t row_id = base + static_cast<uint32_t>(i);
      const auto& maybe_value = buffer[i];
      if (!maybe_value.has_value()) {
         null_bitmap.add(row_id);
         // We need to add something to the vector, so that the size of the vector remains equal to
         // row_id but we do not add our row_id to indexed_values[value_id]
         const Idx value_id = ids_by_value.at("");
         indexed_values.try_emplace(value_id);
         chunk.push_back(value_id);
         continue;
      }
      const Idx value_id = ids_by_value.at(*maybe_value);

      indexed_values[value_id].add(row_id);
      chunk.push_back(value_id);
   }
   value_ids.appendChunk(std::move(chunk));
   return {};
}

void DictionaryEncodedColumn::update(
   const roaring::Roaring& row_ids,
   const std::optional<std::string>& value
) {
   // Null rows carry the empty-string placeholder id as their stored value (see `appendChunk`), so
   // the target id is that placeholder for a null update and the interned value id otherwise.
   const Buffer new_values{value};
   const Idx new_value_id = internValues(new_values).at(value.value_or(""));
   // `try_emplace` keeps the id present in the index even for the null placeholder, matching how
   // `appendChunk` treats null rows (present as a value id, absent from any match bitmap).
   indexed_values.try_emplace(new_value_id);

   for (const uint32_t global_row_id : row_ids) {
      const RowId row_id = RowId::fromGlobal(global_row_id);
      // Detach the row from its previous value's match bitmap. Previously-null rows are not members
      // of any match bitmap, so only non-null rows need detaching.
      if (!null_bitmap.contains(global_row_id)) {
         const Idx old_value_id = value_ids.at(row_id);
         indexed_values.at(old_value_id).remove(global_row_id);
      }
      value_ids.setValue(row_id, new_value_id);
      if (value.has_value()) {
         indexed_values.at(new_value_id).add(global_row_id);
      }
   }

   if (value.has_value()) {
      null_bitmap -= row_ids;
   } else {
      null_bitmap |= row_ids;
   }
}

void DictionaryEncodedColumn::update(
   uint16_t chunk_id,
   roaring_util::RoaringContainerView rows,
   const Buffer& new_values
) {
   RHYDB_ASSERT_EQ(new_values.size(), static_cast<size_t>(rows.getCardinality()));

   const auto ids_by_value = internValues(new_values);
   size_t index = 0;
   for (const uint16_t row_in_chunk : rows) {
      const RowId row_id{.chunk_id = chunk_id, .row_in_chunk = row_in_chunk};
      const uint32_t global_row_id = row_id.toGlobal();
      const std::optional<std::string>& value = new_values[index++];
      // As in the single-value `update`: a null row stores the empty-string placeholder id and is
      // absent from every match bitmap.
      const Idx new_value_id = ids_by_value.at(value.value_or(""));
      indexed_values.try_emplace(new_value_id);

      if (!null_bitmap.contains(global_row_id)) {
         indexed_values.at(value_ids.at(row_id)).remove(global_row_id);
      }
      value_ids.setValue(row_id, new_value_id);
      if (value.has_value()) {
         indexed_values.at(new_value_id).add(global_row_id);
         null_bitmap.remove(global_row_id);
      } else {
         null_bitmap.add(global_row_id);
      }
   }
}

bool DictionaryEncodedColumn::isNull(RowId row_id) const {
   return null_bitmap.contains(row_id.toGlobal());
}

std::optional<rhydb::Idx> DictionaryEncodedColumn::getValueId(const std::string& value) const {
   return metadata->getId(value);
}

}  // namespace rhydb::storage::column
