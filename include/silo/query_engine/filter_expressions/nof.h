#ifndef SILO_NOF_H
#define SILO_NOF_H

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct NOf : public Expression {
  private:
   std::vector<std::unique_ptr<Expression>> children;
   int number_of_matchers;
   bool match_exactly;

   struct map_child_expressions_result {
      std::vector<std::unique_ptr<operators::Operator>> non_negated_child_operators;
      std::vector<std::unique_ptr<operators::Operator>> negated_child_operators;
      int updated_number_of_matchers;
   };

   map_child_expressions_result map_child_expressions(
      const silo::Database& database,
      const silo::DatabasePartition& database_partition
   ) const;

  public:
   explicit NOf(
      std::vector<std::unique_ptr<Expression>>&& children,
      int number_of_matchers,
      bool match_exactly
   );

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_NOF_H
