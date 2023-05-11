#ifndef SILO_DATE_BETWEEN_H
#define SILO_DATE_BETWEEN_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct DateBetween : public Expression {
   time_t date_from;
   bool open_from;
   time_t date_to;
   bool open_to;

   explicit DateBetween(time_t date_from, bool open_from, time_t date_to, bool open_to);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_DATE_BETWEEN_H
