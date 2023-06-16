#ifndef SILO_FLOAT_BETWEEN_H
#define SILO_FLOAT_BETWEEN_H

#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

class FloatBetween : public Expression {
  private:
   std::string column;
   std::optional<double> from;
   std::optional<double> to;

  public:
   explicit FloatBetween(std::string column, std::optional<double> from, std::optional<double> to);

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatBetween>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_FLOAT_BETWEEN_H
