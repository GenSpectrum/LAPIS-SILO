#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

#include <roaring/roaring.hh>

#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/column/column.h"

namespace silo::query_engine::filter::operators {

template <storage::column::Column ColumnType>
class StringInSet : public Predicate {
  public:
   enum class Comparator : uint8_t { IN, NOT_IN };

  private:
   const ColumnType* column;
   std::unordered_set<std::string> values;
   Comparator comparator;

  public:
   explicit StringInSet(
      const ColumnType* column,
      Comparator comparator,
      std::unordered_set<std::string> values
   );

   ~StringInSet() noexcept override;

   [[nodiscard]] std::string toString() const override;
   [[nodiscard]] bool match(uint32_t row_id) const override;

   [[nodiscard]] std::unique_ptr<Predicate> copy() const override;
   [[nodiscard]] std::unique_ptr<Predicate> negate() const override;
};

}  // namespace silo::query_engine::filter::operators
