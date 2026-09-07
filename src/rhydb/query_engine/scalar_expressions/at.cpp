#include "rhydb/query_engine/scalar_expressions/at.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/filter/operators/operator.h"

namespace rhydb::query_engine::scalar_expressions {

At::At(std::unique_ptr<ScalarExpression> input, uint32_t position)
    : input(std::move(input)),
      position(position) {
   RHYDB_ASSERT(this->input != nullptr);
}

std::string At::toString() const {
   return fmt::format("{}.at({})", input->toString(), position);
}

std::vector<schema::ColumnIdentifier> At::freeIUs() const {
   return input->freeIUs();
}

std::unique_ptr<ScalarExpression> At::rewrite(const storage::Table& table, AmbiguityMode mode)
   const {
   return std::make_unique<At>(input->rewrite(table, mode), position);
}

std::unique_ptr<filter::operators::Operator> At::compile(const storage::Table& /*table*/) const {
   // `at` yields a string scalar value, not a filter predicate.
   RHYDB_UNIMPLEMENTED();
}

}  // namespace rhydb::query_engine::scalar_expressions
