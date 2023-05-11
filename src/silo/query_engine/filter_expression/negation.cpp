#include "silo/query_engine/filter_expressions/negation.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/range_selection.h"
#include "silo/query_engine/operators/selection.h"

#include "silo/storage/database_partition.h"

namespace operators = silo::query_engine::operators;

namespace silo::query_engine::filter_expressions {

Negation::Negation(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Negation::toString(const silo::Database& database) {
   std::string result = "!" + child->toString(database);
   return result;
}

std::unique_ptr<operators::Operator> Negation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   std::unique_ptr<operators::Operator> child_operator =
      child->compile(database, database_partition);
   if (child_operator->type() == operators::COMPLEMENT) {
      return std::move(dynamic_cast<operators::Complement*>(child_operator.get())->child);
   } else if (child_operator->type() == operators::RANGE_SELECTION) {
      dynamic_cast<operators::RangeSelection*>(child_operator.get())->negate();
      return child_operator;
   } else if (child_operator->type() == operators::SELECTION) {
      dynamic_cast<operators::Selection*>(child_operator.get())->negate();
      return child_operator;
   } else if (child_operator->type() == operators::EMPTY) {
      return std::make_unique<operators::Full>(database_partition.sequenceCount);
   } else if (child_operator->type() == operators::FULL) {
      return std::make_unique<operators::Empty>();
   } else if (child_operator->type() == operators::INTERSECTION) {
      return std::make_unique<operators::Empty>();
   } else {
      return std::make_unique<operators::Complement>(
         std::move(child_operator), database_partition.sequenceCount
      );
   }
}

}  // namespace silo::query_engine::filter_expressions