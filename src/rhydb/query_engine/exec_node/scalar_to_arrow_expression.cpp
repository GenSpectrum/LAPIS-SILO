#include "rhydb/query_engine/exec_node/scalar_to_arrow_expression.h"

#include <cstdint>

#include <arrow/compute/api.h>
#include <arrow/datum.h>

#include "rhydb/query_engine/exec_node/zstd_decompress_expression.h"
#include "rhydb/query_engine/scalar_expressions/at.h"
#include "rhydb/query_engine/scalar_expressions/field_ref.h"
#include "rhydb/query_engine/scalar_expressions/iso_week.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/query_engine/scalar_expressions/zstd_decompress_scalar.h"

namespace rhydb::query_engine::exec_node {

using scalar_expressions::At;
using scalar_expressions::BoolLiteral;
using scalar_expressions::dynCast;
using scalar_expressions::FieldRef;
using scalar_expressions::FloatLiteral;
using scalar_expressions::Int32Literal;
using scalar_expressions::Int64Literal;
using scalar_expressions::IsoWeek;
using scalar_expressions::ScalarExpression;
using scalar_expressions::StringLiteral;
using scalar_expressions::ZstdDecompressScalar;

// NOLINTNEXTLINE(misc-no-recursion)
arrow::Result<arrow::compute::Expression> scalarToArrowExpression(const ScalarExpression& expression
) {
   if (const auto* literal = dynCast<Int32Literal>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynCast<Int64Literal>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynCast<FloatLiteral>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynCast<StringLiteral>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* literal = dynCast<BoolLiteral>(&expression)) {
      return arrow::compute::literal(arrow::Datum(literal->value));
   }
   if (const auto* field_ref = dynCast<FieldRef>(&expression)) {
      return arrow::compute::field_ref(field_ref->column.name);
   }
   if (const auto* zstd = dynCast<ZstdDecompressScalar>(&expression)) {
      ARROW_ASSIGN_OR_RAISE(auto input, scalarToArrowExpression(*zstd->input));
      return ZstdDecompressExpression::make(input, zstd->dictionary_string);
   }
   if (const auto* at_function = dynCast<At>(&expression)) {
      // `at` is 1-indexed; utf8_slice_codeunits takes a 0-indexed, half-open
      // [start, stop) range of code units, so extract the single character at
      // position-1.
      const int64_t start = static_cast<int64_t>(at_function->position) - 1;
      const auto stop = static_cast<int64_t>(at_function->position);
      ARROW_ASSIGN_OR_RAISE(auto input, scalarToArrowExpression(*at_function->input));
      return arrow::compute::call(
         "utf8_slice_codeunits", {input}, arrow::compute::SliceOptions(start, stop)
      );
   }
   if (const auto* iso_week = dynCast<IsoWeek>(&expression)) {
      ARROW_ASSIGN_OR_RAISE(auto input, scalarToArrowExpression(*iso_week->input));
      return arrow::compute::call("iso_week", {input});
   }
   return arrow::Status::NotImplemented(
      "the scalar expression ", expression.toString(), " is not supported in map() assignments"
   );
}

}  // namespace rhydb::query_engine::exec_node
