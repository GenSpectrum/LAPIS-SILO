#include "silo/query_engine/scalar_expressions/at.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::scalar_expressions {

At::At(schema::ColumnIdentifier input_column, uint32_t position)
    : input_column(std::move(input_column)),
      position(position) {}

std::string At::toString() const {
   return fmt::format("{}.at({})", input_column.name, position);
}

std::vector<schema::ColumnIdentifier> At::freeIUs() const {
   return {input_column};
}

std::unique_ptr<ScalarExpression> At::
   rewrite(const storage::Table& /*table*/, AmbiguityMode /*mode*/) const {
   return std::make_unique<At>(input_column, position);
}

std::unique_ptr<filter::operators::Operator> At::compile(const storage::Table& /*table*/) const {
   // `at` yields a string scalar value, not a filter predicate.
   SILO_UNIMPLEMENTED();
}

}  // namespace silo::query_engine::scalar_expressions
