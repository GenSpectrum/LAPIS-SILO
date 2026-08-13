#include "silo/query_engine/scalar_expressions/iso_week.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace rhydb::query_engine::scalar_expressions {

IsoWeek::IsoWeek(std::unique_ptr<ScalarExpression> input)
    : input(std::move(input)) {
   SILO_ASSERT(this->input != nullptr);
}

std::string IsoWeek::toString() const {
   return fmt::format("{}.isoWeek()", input->toString());
}

std::vector<schema::ColumnIdentifier> IsoWeek::freeIUs() const {
   return input->freeIUs();
}

std::unique_ptr<ScalarExpression> IsoWeek::rewrite(const storage::Table& table, AmbiguityMode mode)
   const {
   return std::make_unique<IsoWeek>(input->rewrite(table, mode));
}

std::unique_ptr<filter::operators::Operator> IsoWeek::compile(const storage::Table& /*table*/)
   const {
   SILO_UNIMPLEMENTED();
}

}  // namespace rhydb::query_engine::scalar_expressions
