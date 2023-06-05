#ifndef SILO_AND_H
#define SILO_AND_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct And : public Expression {
  private:
   std::vector<std::unique_ptr<Expression>> children;

   std::tuple<
      std::vector<std::unique_ptr<operators::Operator>>,
      std::vector<std::unique_ptr<operators::Operator>>>
   compileChildren(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const;

  public:
   explicit And(std::vector<std::unique_ptr<Expression>>&& children);

   std::string toString(const silo::Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<And>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_AND_H
