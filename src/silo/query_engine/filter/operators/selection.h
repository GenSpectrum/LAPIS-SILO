#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/german_string.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/string_column.h"

namespace silo::query_engine::filter::expressions {
class And;
}

namespace silo::query_engine::filter::operators {

class Predicate {
  public:
   virtual ~Predicate() noexcept = default;

   [[nodiscard]] virtual std::string toString() const = 0;
   [[nodiscard]] virtual bool match(uint32_t row_id) const = 0;
   [[nodiscard]] virtual std::unique_ptr<Predicate> copy() const = 0;
   [[nodiscard]] virtual std::unique_ptr<Predicate> negate() const = 0;
};

using PredicateVector = std::vector<std::unique_ptr<Predicate>>;

enum class Comparator { EQUALS, LESS, HIGHER, LESS_OR_EQUALS, HIGHER_OR_EQUALS, NOT_EQUALS };

inline std::string displayComparator(Comparator comparator) {
   switch (comparator) {
      case Comparator::EQUALS:
         return "=";
      case Comparator::NOT_EQUALS:
         return "!=";
      case Comparator::LESS:
         return "<";
      case Comparator::HIGHER:
         return ">";
      case Comparator::LESS_OR_EQUALS:
         return "<=";
      case Comparator::HIGHER_OR_EQUALS:
         return ">=";
   }
   SILO_UNREACHABLE();
}

template <storage::column::Column ColumnType>
class CompareToValueSelection : public Predicate {
   const ColumnType& column;
   Comparator comparator;
   ColumnType::value_type value;
   bool with_nulls;

  public:
   CompareToValueSelection(
      const ColumnType& column,
      Comparator comparator,
      ColumnType::value_type value,
      bool with_nulls
   )
       : column(column),
         comparator(comparator),
         value(value),
         with_nulls(with_nulls) {}

   CompareToValueSelection(
      const ColumnType& column,
      Comparator comparator,
      ColumnType::value_type value
   )
       : CompareToValueSelection(column, comparator, value, false) {}

   [[nodiscard]] std::string toString() const override {
      return fmt::format(
         "${} {} {} {}",
         schema::columnTypeToString(ColumnType::TYPE),
         column.metadata->column_name,
         displayComparator(comparator),
         value
      );
   }

   [[nodiscard]] bool match(uint32_t row_id) const override {
      if (column.isNull(row_id)) {
         return with_nulls;
      }
      switch (comparator) {
         case Comparator::EQUALS:
            return column.getValue(row_id) == value;
         case Comparator::NOT_EQUALS:
            return column.getValue(row_id) != value;
         case Comparator::LESS:
            return column.getValue(row_id) < value;
         case Comparator::HIGHER_OR_EQUALS:
            return column.getValue(row_id) >= value;
         case Comparator::HIGHER:
            return column.getValue(row_id) > value;
         case Comparator::LESS_OR_EQUALS:
            return column.getValue(row_id) <= value;
      }
      SILO_UNREACHABLE();
   }

   [[nodiscard]] std::unique_ptr<Predicate> copy() const override {
      return std::make_unique<CompareToValueSelection<ColumnType>>(column, comparator, value);
   }

   [[nodiscard]] std::unique_ptr<Predicate> negate() const override {
      switch (comparator) {
         case Comparator::EQUALS:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::NOT_EQUALS, value, !with_nulls
            );
         case Comparator::NOT_EQUALS:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::EQUALS, value, !with_nulls
            );
         case Comparator::LESS:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::HIGHER_OR_EQUALS, value, !with_nulls
            );
         case Comparator::HIGHER_OR_EQUALS:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::LESS, value, !with_nulls
            );
         case Comparator::HIGHER:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::LESS_OR_EQUALS, value, !with_nulls
            );
         case Comparator::LESS_OR_EQUALS:
            return std::make_unique<CompareToValueSelection>(
               column, Comparator::HIGHER, value, !with_nulls
            );
      }
      SILO_UNREACHABLE();
   }
};

template <>
bool CompareToValueSelection<silo::storage::column::StringColumnPartition>::match(uint32_t row_id
) const;

class Selection : public Operator {
   friend class filter::expressions::And;

  private:
   std::optional<std::unique_ptr<Operator>> child_operator;
   std::vector<std::unique_ptr<Predicate>> predicates;
   uint32_t row_count;

  public:
   Selection(
      std::unique_ptr<Operator>&& child_operator,
      std::vector<std::unique_ptr<Predicate>>&& predicates,
      uint32_t row_count
   );

   Selection(
      std::unique_ptr<Operator>&& child_operator,
      std::unique_ptr<Predicate> predicate,
      uint32_t row_count
   );

   Selection(std::vector<std::unique_ptr<Predicate>>&& predicates, uint32_t row_count);

   Selection(std::unique_ptr<Predicate> predicate, uint32_t row_count);

   ~Selection() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Selection>&& selection);

  private:
   [[nodiscard]] virtual bool matchesPredicates(uint32_t row) const;
};

}  // namespace silo::query_engine::filter::operators
