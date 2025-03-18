//! Runtime-defined compact tuple for one row of the database, for the
//! columns requested by the user.

//! Used for:

//! - index in hash tables for aggregation (hashing and comparison (via memcmp))
//! - currently sorting for order by queries in Details (via TupleFactory)

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "silo/common/json_value_type.h"
#include "silo/storage/column_group.h"

namespace silo::query_engine::actions {

struct OrderByField;

size_t getTupleSize(const std::vector<silo::schema::ColumnIdentifier>& column_list);

class Tuple {
   friend class TupleFactory;

   struct ComparatorField {
      size_t offset;
      silo::schema::ColumnIdentifier type;
      bool ascending;
   };

   const silo::storage::ColumnPartitionGroup* columns;
   std::byte* data;
   size_t data_size;

   static std::vector<ComparatorField> getCompareFields(
      const std::vector<silo::schema::ColumnIdentifier>& columns_metadata,
      const std::vector<OrderByField>& order_by_fields
   );

   /// Compare tuples according to user-provided fields
   [[nodiscard]] bool compareLess(const Tuple& other, const std::vector<ComparatorField>& fields)
      const;

  public:
   typedef std::function<bool(const Tuple&, const Tuple&)> Comparator;

   Tuple(const silo::storage::ColumnPartitionGroup* columns, std::byte* data, size_t data_size);
   Tuple(Tuple&& other) noexcept;
   Tuple& operator=(const Tuple& other);
   Tuple& operator=(Tuple&& other) noexcept;

   [[nodiscard]] std::map<std::string, common::JsonValueType> getFields() const;

   static Comparator getComparator(
      const std::vector<silo::schema::ColumnIdentifier>& column_identifiers,
      const std::vector<OrderByField>& order_by_fields,
      const std::optional<uint32_t>& randomize_seed
   );

   bool operator==(const Tuple& other) const;
   bool operator!=(const Tuple& other) const;

   /// Compare according to native column order.
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
      const std::vector<silo::schema::ColumnIdentifier>& fields
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
