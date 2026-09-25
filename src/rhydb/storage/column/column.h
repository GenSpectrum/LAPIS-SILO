#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace rhydb::schema {
enum class ColumnType : uint8_t;
enum class ValueType : uint8_t;
}  // namespace rhydb::schema

namespace rhydb::storage::column {

/// During ingestion, data is buffered into fixed-size chunks of at most this
/// many rows. A column builder accumulates one such chunk and the resulting
/// value array is applied to the column in a single bulk insert.
static constexpr size_t COLUMN_CHUNK_SIZE = 1UL << 16;

template <typename T>
concept Column = requires(T column) {
   typename T::Metadata;
   // ensure it is actually a type
   requires std::is_class_v<typename T::Metadata> ||
               std::is_same_v<typename T::Metadata, typename T::Metadata>;

   requires std::is_constructible_v<T, typename T::Metadata*>;

   typename T::value_type;

   // A column stores its values as the immutable chunks in which they were ingested. Iterating the
   // rows themselves is the table's job (it owns the shared `RowLayout`); a column only reports how
   // many chunks it holds so the table can check every column was appended to in lockstep.
   { column.numChunks() } -> std::convertible_to<std::size_t>;

   { column.chunkSize(static_cast<uint32_t>(0)) } -> std::convertible_to<uint32_t>;

   { T::TYPE } -> std::convertible_to<schema::ColumnType>;

   // The logical type of the values this column holds (e.g. STRING, INT32, NUCLEOTIDE_SEQUENCE).
   { column.type() } -> std::convertible_to<schema::ValueType>;
};

}  // namespace rhydb::storage::column
