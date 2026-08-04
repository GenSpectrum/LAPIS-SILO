#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/chunked_value_buffer.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/row_id.h"

namespace silo::storage::column {

/// Maps a C++ integer value type to its schema::ColumnType. Adding a further width (e.g. int16_t or
/// an unsigned variant) only requires a new specialization here plus the corresponding
/// schema::ColumnType enum value and config/query wiring.
template <typename T>
struct NumericColumnTypeTraits;

template <>
struct NumericColumnTypeTraits<int32_t> {
   static constexpr schema::ColumnType TYPE = schema::ColumnType::INT32;
};

template <>
struct NumericColumnTypeTraits<int64_t> {
   static constexpr schema::ColumnType TYPE = schema::ColumnType::INT64;
};

template <typename T>
class NumericColumnBuilder;

/// A fixed-width integer column storing values of type T. Nulls are tracked in a roaring bitmap;
/// the value slot of a null row holds 0. Instantiated as IntColumn (int32_t) and Int64Column
/// (int64_t) via type aliases.
template <typename T>
class NumericColumn {
  public:
   using Metadata = ColumnMetadata;
   using Builder = NumericColumnBuilder<T>;
   using Buffer = std::vector<std::optional<T>>;

   static constexpr schema::ColumnType TYPE = NumericColumnTypeTraits<T>::TYPE;
   using value_type = T;

  private:
   ChunkedValueBuffer<T> values;

  public:
   roaring::Roaring null_bitmap;

   Metadata* metadata;

   explicit NumericColumn(Metadata* metadata)
       : metadata(metadata) {}

   [[nodiscard]] bool isNull(RowId row_id) const { return null_bitmap.contains(row_id.toGlobal()); }

   [[nodiscard]] T getValue(RowId row_id) const {
      SILO_ASSERT(!null_bitmap.contains(row_id.toGlobal()));
      return values.at(row_id);
   }

   [[nodiscard]] size_t numChunks() const { return values.numChunks(); }

   [[nodiscard]] uint32_t chunkSize(uint16_t chunk_id) const { return values.chunkSize(chunk_id); }

   std::expected<void, std::string> appendChunk(const Buffer& buffer) {
      const uint32_t base = RowId::chunkStart(static_cast<uint16_t>(values.numChunks()));
      std::vector<T> chunk;
      chunk.reserve(buffer.size());
      for (size_t i = 0; i < buffer.size(); ++i) {
         if (buffer[i].has_value()) {
            chunk.push_back(*buffer[i]);
         } else {
            null_bitmap.add(base + static_cast<uint32_t>(i));
            chunk.push_back(0);
         }
      }
      values.appendChunk(std::move(chunk));
      return {};
   }

   /// Assigns `value` to every row in `row_ids` (physical global row ids). A `std::nullopt` value
   /// marks the rows null; a concrete value clears their null flag and overwrites the stored value
   /// in place. Rows not in `row_ids` are left untouched.
   void update(const roaring::Roaring& row_ids, std::optional<T> value) {
      if (value == std::nullopt) {
         null_bitmap |= row_ids;
      } else {
         null_bitmap -= row_ids;
      }
      const T stored_value = value.value_or(0);
      for (const uint32_t global_row_id : row_ids) {
         values.setValue(RowId::fromGlobal(global_row_id), stored_value);
      }
   }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & values;
      archive & null_bitmap;
      // clang-format on
   }
};

template <typename T>
class NumericColumnBuilder {
   NumericColumn<T>::Buffer buffer;

  public:
   void insert(T value) { buffer.emplace_back(value); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] NumericColumn<T>::Buffer finalize() {
      typename NumericColumn<T>::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column
