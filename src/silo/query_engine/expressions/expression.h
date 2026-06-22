#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::expressions {

class Expression {
  protected:
   Expression();

  public:
   virtual ~Expression() = default;

   /// UPPER_BOUND returns the upper bound of sequences matching this expression (i.e. ambiguous
   /// codes count as matches), LOWER_BOUND returns the lower bound of sequences matching this
   /// expression (i.e. ambiguous codes in negations count as matches)
   /// NONE does not specially consider ambiguous symbols
   enum AmbiguityMode : uint8_t { UPPER_BOUND, LOWER_BOUND, NONE };

   // For clean type-checks
   enum class Kind : uint8_t {
      AND,
      OR,
      N_OF,
      NEGATION,
      MAYBE,
      EXACT,
      BOOL_EQUALS,
      DATE_BETWEEN,
      DATE_EQUALS,
      FIELD_REF,
      FLOAT_BETWEEN,
      FLOAT_EQUALS,
      INT_BETWEEN,
      INT_EQUALS,
      IS_NULL,
      LINEAGE_FILTER,
      PHYLO_CHILD_FILTER,
      STRING_EQUALS,
      STRING_IN_SET,
      STRING_SEARCH,
      INT64_LITERAL,
      FLOAT_LITERAL,
      STRING_LITERAL,
      BOOL_LITERAL,
      HAS_MUTATION_NUCLEOTIDE,
      HAS_MUTATION_AMINO_ACID,
      INSERTION_CONTAINS_NUCLEOTIDE,
      INSERTION_CONTAINS_AMINO_ACID,
      MUTATION_PROFILE_NUCLEOTIDE,
      MUTATION_PROFILE_AMINO_ACID,
      SYMBOL_EQUALS_NUCLEOTIDE,
      SYMBOL_EQUALS_AMINO_ACID,
      SYMBOL_IN_SET_NUCLEOTIDE,
      SYMBOL_IN_SET_AMINO_ACID,
   };

   /// The column type this expression evaluates to. All current expressions are
   /// boolean filter predicates; non-boolean scalar expressions may override this.
   [[nodiscard]] virtual schema::ColumnType type() const { return schema::ColumnType::BOOL; }

   /// The concrete type of this expression, used by isA<>/dynCast<>.
   [[nodiscard]] virtual Kind kind() const = 0;

   [[nodiscard]] virtual std::string toString() const = 0;

   /// The columns ("identifiable units") this expression references and that an
   /// upstream node must therefore provide. Literals reference none; a column
   /// reference yields that column. Used by column narrowing to keep the child
   /// columns a scalar expression depends on alive.
   [[nodiscard]] virtual std::vector<schema::ColumnIdentifier> freeIUs() const { return {}; }

   [[nodiscard]] virtual std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const = 0;

   [[nodiscard]] virtual std::unique_ptr<Expression> clone() const = 0;

   [[nodiscard]] virtual std::unique_ptr<filter::operators::Operator> compile(
      const storage::Table& table
   ) const = 0;
};

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode);

/// `To` must expose a `static constexpr Kind KIND` identifying its concrete type
template <typename To, typename From>
[[nodiscard]] bool isA(const From* expression) {
   return expression != nullptr && expression->kind() == To::KIND;
}

template <typename To, typename From>
[[nodiscard]] const To* dynCast(const From* expression) {
   return isA<To>(expression) ? static_cast<const To*>(expression) : nullptr;
}

template <typename To, typename From>
[[nodiscard]] To* dynCast(From* expression) {
   return isA<To>(expression) ? static_cast<To*>(expression) : nullptr;
}

template <typename T>
void appendVectorToVector(
   std::vector<std::unique_ptr<T>>& vec_1,
   std::vector<std::unique_ptr<T>>& vec_2
) {
   std::ranges::transform(vec_1, std::back_inserter(vec_2), [&](std::unique_ptr<T>& ele) {
      return std::move(ele);
   });
}

using ExpressionVector = std::vector<std::unique_ptr<Expression>>;

}  // namespace silo::query_engine::expressions
