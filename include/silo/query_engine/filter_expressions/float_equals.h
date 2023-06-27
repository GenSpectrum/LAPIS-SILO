#ifndef SILO_FLOAT_EQUALS_H
#define SILO_FLOAT_EQUALS_H

#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

class FloatEquals : public Expression {
  private:
   std::string column;
   double value;

  public:
   FloatEquals(std::string column, double value);

   std::string toString(const silo::Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatEquals>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_FLOAT_EQUALS_H
