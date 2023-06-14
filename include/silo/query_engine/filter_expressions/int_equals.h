#ifndef SILO_INT_EQUALS_H
#define SILO_INT_EQUALS_H

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct IntEquals : public Expression {
  private:
   std::string column;
   uint64_t value;

  public:
   explicit IntEquals(std::string column, uint64_t value);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntEquals>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_INT_EQUALS_H
