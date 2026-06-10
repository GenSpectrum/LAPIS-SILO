#include "silo/query_engine/expressions/literal.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

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

bool Int64Literal::operator==(const Expression& other) const {
   const auto* other_casted = dynamic_cast<const Int64Literal*>(&other);
   return other_casted != nullptr && value == other_casted->value;
}

std::unique_ptr<Expression> Int64Literal::rewrite(
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

bool FloatLiteral::operator==(const Expression& other) const {
   const auto* other_casted = dynamic_cast<const FloatLiteral*>(&other);
   return other_casted != nullptr && value == other_casted->value;
}

std::unique_ptr<Expression> FloatLiteral::rewrite(
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

bool StringLiteral::operator==(const Expression& other) const {
   const auto* other_casted = dynamic_cast<const StringLiteral*>(&other);
   return other_casted != nullptr && value == other_casted->value;
}

std::unique_ptr<Expression> StringLiteral::rewrite(
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

bool BoolLiteral::operator==(const Expression& other) const {
   const auto* other_casted = dynamic_cast<const BoolLiteral*>(&other);
   return other_casted != nullptr && value == other_casted->value;
}

std::unique_ptr<Expression> BoolLiteral::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<BoolLiteral>(value);
}

std::unique_ptr<filter::operators::Operator> BoolLiteral::compile(const storage::Table& table
) const {
   if (value) {
      return std::make_unique<filter::operators::Full>(table.sequence_count);
   }
   return std::make_unique<filter::operators::Empty>(table.sequence_count);
}

}  // namespace silo::query_engine::expressions
