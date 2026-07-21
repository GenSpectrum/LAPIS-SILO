#include "silo/query_engine/scalar_expressions/literal.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::scalar_expressions {

namespace {
/// Literals are scalar values, not filter predicates. Only the boolean literal
/// has a meaningful compilation into a filter operator.
[[noreturn]] std::unique_ptr<filter::operators::Operator> compileNonBooleanLiteral() {
   SILO_UNIMPLEMENTED();
}
}  // namespace

Int64Literal::Int64Literal(int64_t value)
    : value(value) {}

std::string Int64Literal::toString() const {
   return fmt::format("{}", value);
}

std::unique_ptr<ScalarExpression> Int64Literal::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<Int64Literal>(value);
}

std::unique_ptr<filter::operators::Operator> Int64Literal::compile(const storage::Table& /*table*/)
   const {
   compileNonBooleanLiteral();
}

FloatLiteral::FloatLiteral(double value)
    : value(value) {}

std::string FloatLiteral::toString() const {
   return fmt::format("{}", value);
}

std::unique_ptr<ScalarExpression> FloatLiteral::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatLiteral>(value);
}

std::unique_ptr<filter::operators::Operator> FloatLiteral::compile(const storage::Table& /*table*/)
   const {
   compileNonBooleanLiteral();
}

StringLiteral::StringLiteral(std::string value)
    : value(std::move(value)) {}

std::string StringLiteral::toString() const {
   return fmt::format("'{}'", value);
}

std::unique_ptr<ScalarExpression> StringLiteral::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<StringLiteral>(value);
}

std::unique_ptr<filter::operators::Operator> StringLiteral::compile(const storage::Table& /*table*/)
   const {
   compileNonBooleanLiteral();
}

BoolLiteral::BoolLiteral(bool value)
    : value(value) {}

std::string BoolLiteral::toString() const {
   return value ? "true" : "false";
}

std::unique_ptr<ScalarExpression> BoolLiteral::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<BoolLiteral>(value);
}

std::unique_ptr<filter::operators::Operator> BoolLiteral::compile(const storage::Table& table
) const {
   if (value) {
      return std::make_unique<filter::operators::Full>(table.row_layout);
   }
   return std::make_unique<filter::operators::Empty>(table.row_layout);
}

}  // namespace silo::query_engine::scalar_expressions
