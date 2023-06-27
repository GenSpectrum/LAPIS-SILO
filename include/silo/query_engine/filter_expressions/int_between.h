#ifndef SILO_INT_BETWEEN_H
#define SILO_INT_BETWEEN_H

#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct IntBetween : public Expression {
  private:
   std::string column;
   std::optional<uint64_t> from;
   std::optional<uint64_t> to;

  public:
   explicit IntBetween(
      std::string column,
      std::optional<uint64_t> from,
      std::optional<uint64_t> to
   );

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntBetween>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_INT_BETWEEN_H
