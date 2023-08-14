#ifndef SILO_TUPLE_H
#define SILO_TUPLE_H

#include <optional>
#include <string>

#include "silo/common/date.h"
#include "silo/common/string.h"
#include "silo/storage/column_group.h"

namespace silo::query_engine::actions {

struct OrderByField;

size_t getTupleSize(const std::vector<silo::storage::ColumnMetadata>& group_by_metadata);

struct TupleFieldComparator {
   size_t offset;
   silo::storage::ColumnMetadata type;
   bool ascending;
};

struct Tuple {
   typedef std::function<bool(const Tuple&, const Tuple&)> Comparator;

   const silo::storage::ColumnPartitionGroup* columns;
   std::vector<char> data;

   Tuple(
      uint32_t sequence_id,
      const silo::storage::ColumnPartitionGroup* columns,
      size_t tuple_size
   );

   Tuple() = default;
   Tuple(const Tuple& other);
   Tuple(Tuple&& other);
   Tuple& operator=(const Tuple& other);
   Tuple& operator=(Tuple&& other);

   [[nodiscard]] std::map<std::string, std::optional<std::variant<std::string, int32_t, double>>>
   getFields() const;

   static std::vector<TupleFieldComparator> getCompareFields(
      const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
      const std::vector<OrderByField>& order_by_fields
   );
   static Comparator getComparator(
      const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
      const std::vector<OrderByField>& order_by_fields
   );

   bool compareLess(const Tuple& other, const std::vector<TupleFieldComparator>& fields) const;

   bool operator==(const Tuple& other) const;
   bool operator!=(const Tuple& other) const;

   bool operator<(const Tuple& other) const;
   bool operator>(const Tuple& other) const;
   bool operator<=(const Tuple& other) const;
   bool operator>=(const Tuple& other) const;
};

}  // namespace silo::query_engine::actions

template <>
struct std::hash<silo::query_engine::actions::Tuple> {
   std::size_t operator()(const silo::query_engine::actions::Tuple& tuple) const {
      const std::string_view str_view(tuple.data.data(), tuple.data.size());
      return std::hash<std::string_view>{}(str_view);
   }
};

#endif  // SILO_TUPLE_H
