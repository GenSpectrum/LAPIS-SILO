#pragma once

#include <optional>
#include <string>

#include "silo/common/date.h"
#include "silo/common/string.h"
#include "silo/storage/column_group.h"

namespace silo::query_engine::actions {

struct OrderByField;

size_t getTupleSize(const std::vector<silo::storage::ColumnMetadata>& metadata_list);

class Tuple {
   friend class TupleFactory;

   struct ComparatorField {
      size_t offset;
      silo::storage::ColumnMetadata type;
      bool ascending;
   };

   const silo::storage::ColumnPartitionGroup* columns;
   std::byte* data;
   size_t data_size;

   static std::vector<ComparatorField> getCompareFields(
      const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
      const std::vector<OrderByField>& order_by_fields
   );

   [[nodiscard]] bool compareLess(const Tuple& other, const std::vector<ComparatorField>& fields)
      const;

  public:
   typedef std::function<bool(const Tuple&, const Tuple&)> Comparator;

   Tuple(const silo::storage::ColumnPartitionGroup* columns, std::byte* data, size_t data_size);
   Tuple(Tuple&& other) noexcept;
   Tuple& operator=(const Tuple& other);
   Tuple& operator=(Tuple&& other) noexcept;

   [[nodiscard]] std::map<std::string, std::optional<std::variant<std::string, int32_t, double>>>
   getFields() const;

   static Comparator getComparator(
      const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
      const std::vector<OrderByField>& order_by_fields
   );

   bool operator==(const Tuple& other) const;
   bool operator!=(const Tuple& other) const;

   bool operator<(const Tuple& other) const;
   bool operator>(const Tuple& other) const;
   bool operator<=(const Tuple& other) const;
   bool operator>=(const Tuple& other) const;

   friend std::hash<Tuple>;
};

}  // namespace silo::query_engine::actions

template <>
struct std::hash<silo::query_engine::actions::Tuple> {
   std::size_t operator()(const silo::query_engine::actions::Tuple& tuple) const;
};

namespace silo::query_engine::actions {

class TupleFactory {
   std::deque<std::vector<std::byte>> all_tuple_data;
   silo::storage::ColumnPartitionGroup columns;
   size_t tuple_size;

  public:
   explicit TupleFactory(
      const silo::storage::ColumnPartitionGroup& all_columns,
      const std::vector<silo::storage::ColumnMetadata>& fields
   );

   Tuple allocateOne(uint32_t sequence_id);

   Tuple& overwrite(Tuple& tuple, uint32_t sequence_id);

   Tuple copyTuple(const Tuple& tuple);

   /// The vector will contain null-initialized Tuples.
   /// The caller needs to guarantee that these Tuples will be overwritten using the
   /// TupleFactory::overwrite method, before any member function is called
   /// on them.
   std::vector<Tuple> allocateMany(size_t count);
};

}  // namespace silo::query_engine::actions
