#ifndef SILO_DATE_BETWEEN_H
#define SILO_DATE_BETWEEN_H

#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct DateBetween : public Expression {
   std::optional<time_t> date_from;
   std::optional<time_t> date_to;

   explicit DateBetween(std::optional<time_t> date_from, std::optional<time_t> date_to);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_DATE_BETWEEN_H
