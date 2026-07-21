#include "silo/query_engine/scalar_expressions/iso_week.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::scalar_expressions {

IsoWeek::IsoWeek(schema::ColumnIdentifier input_column)
    : input_column(std::move(input_column)) {}

std::string IsoWeek::toString() const {
   return fmt::format("{}.isoWeek()", input_column.name);
}

std::vector<schema::ColumnIdentifier> IsoWeek::freeIUs() const {
   return {input_column};
}

std::unique_ptr<ScalarExpression> IsoWeek::
   rewrite(const storage::Table& /*table*/, AmbiguityMode /*mode*/) const {
   return std::make_unique<IsoWeek>(input_column);
}

std::unique_ptr<filter::operators::Operator> IsoWeek::compile(const storage::Table& /*table*/)
   const {
   // `isoWeek` yields an int scalar value, not a filter predicate.
   SILO_UNIMPLEMENTED();
}

}  // namespace silo::query_engine::scalar_expressions
