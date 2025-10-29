#pragma once

#include <cstdint>
#include <type_traits>

namespace silo::schema {
enum class ColumnType : uint8_t;
}

namespace silo::storage::column {

template <typename T>
concept Column = requires(T t) {
   typename T::Metadata;
   // ensure it is actually a type
   requires std::is_class_v<typename T::Metadata> ||
               std::is_same_v<typename T::Metadata, typename T::Metadata>;

   requires std::is_constructible_v<T, typename T::Metadata*>;

   { t.numValues() } -> std::convertible_to<size_t>;

   { T::TYPE } -> std::convertible_to<schema::ColumnType>;
};

}  // namespace silo::storage::column
