#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/string.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
struct And;
}

namespace silo::query_engine::operators {

class Predicate {
  public:
   virtual ~Predicate() noexcept = default;

   [[nodiscard]] virtual std::string toString() const = 0;
   [[nodiscard]] virtual bool match(uint32_t row_id) const = 0;
   [[nodiscard]] virtual std::unique_ptr<Predicate> copy() const = 0;
   [[nodiscard]] virtual std::unique_ptr<Predicate> negate() const = 0;
};

enum class Comparator { EQUALS, LESS, HIGHER, LESS_OR_EQUALS, HIGHER_OR_EQUALS, NOT_EQUALS };

template <typename T>
class CompareToValueSelection : public Predicate {
   const std::vector<T>& column;
   Comparator comparator;
   T value;

  public:
   explicit CompareToValueSelection(const std::vector<T>& column, Comparator comparator, T value);

   [[nodiscard]] std::string toString() const override;
   [[nodiscard]] bool match(uint32_t row_id) const override;
   [[nodiscard]] std::unique_ptr<Predicate> copy() const override;
   [[nodiscard]] std::unique_ptr<Predicate> negate() const override;
};

class Selection : public Operator {
   friend class filter_expressions::And;

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

   [[nodiscard]] OperatorResult evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Selection>&& selection);

  private:
   [[nodiscard]] virtual bool matchesPredicates(uint32_t row) const;
};

}  // namespace silo::query_engine::operators
