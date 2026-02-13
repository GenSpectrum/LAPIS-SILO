#include "silo/query_engine/filter/operators/string_in_set.h"

#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/string_column.h"

namespace silo::query_engine::filter::operators {

using storage::column::Column;

template <Column ColumnType>
StringInSet<ColumnType>::StringInSet(
   const ColumnType* column,
   Comparator comparator,
   std::unordered_set<std::string> values
)
    : column(column),
      values(std::move(values)),
      comparator(comparator) {}

template <Column ColumnType>
StringInSet<ColumnType>::~StringInSet() noexcept = default;

template <Column ColumnType>
std::string StringInSet<ColumnType>::toString() const {
   return fmt::format(
      "{} {} [{}]",
      column->metadata->column_name,
      comparator == Comparator::IN ? "IN" : "NOT IN",
      fmt::join(values, ",")
   );
}

template <Column ColumnType>
bool StringInSet<ColumnType>::match(uint32_t row_id) const {
   const bool in_set = values.contains(column->getValueString(row_id));
   return comparator == Comparator::IN ? in_set : !in_set;
}

template <Column ColumnType>
std::unique_ptr<Predicate> StringInSet<ColumnType>::copy() const {
   return std::make_unique<operators::StringInSet<ColumnType>>(column, comparator, values);
}

template <Column ColumnType>
std::unique_ptr<Predicate> StringInSet<ColumnType>::negate() const {
   const Comparator negated_comparator = comparator == Comparator::IN
                                            ? StringInSet<ColumnType>::Comparator::NOT_IN
                                            : StringInSet<ColumnType>::Comparator::IN;
   return std::make_unique<StringInSet<ColumnType>>(column, negated_comparator, values);
}

template class StringInSet<storage::column::StringColumnPartition>;
template class StringInSet<storage::column::IndexedStringColumnPartition>;

}  // namespace silo::query_engine::filter::operators
