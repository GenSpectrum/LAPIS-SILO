#include "rhydb/query_engine/exec_node/scalar_to_arrow_expression.h"

#include <cstdint>

#include <arrow/compute/api.h>
#include <arrow/datum.h>
#include <arrow/type.h>

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
      // Render the ISO 8601 week date `<ISO-year>-W<ISO-week>`, e.g. `2026-W12`, from the ISO
      // week-numbering year and week. Built from `iso_year` / `iso_week` (both operate on the naive
      // date) rather than `strftime`: strftime's kernel needs the IANA timezone database at runtime
      // -- it looks up `UTC` even for a timezone-naive date32 -- which is not present in every
      // deployment (minimal containers, wasm, ...) and fails there with "Cannot locate ... 'UTC'".
      const auto year_string = arrow::compute::call(
         "cast",
         {arrow::compute::call("iso_year", {input})},
         arrow::compute::CastOptions::Safe(arrow::utf8())
      );
      const auto week_string = arrow::compute::call(
         "cast",
         {arrow::compute::call("iso_week", {input})},
         arrow::compute::CastOptions::Safe(arrow::utf8())
      );
      // Zero-pad the week to two digits so `2021-W02` sorts before `2021-W10`.
      const auto week_padded =
         arrow::compute::call("utf8_lpad", {week_string}, arrow::compute::PadOptions(2, "0"));
      // Join `<year> + "-W" + <week>` -- for binary_join_element_wise the last argument is the
      // separator. A null date propagates through iso_year/iso_week to a null result.
      return arrow::compute::call(
         "binary_join_element_wise", {year_string, week_padded, arrow::compute::literal("-W")}
      );
   }
   return arrow::Status::NotImplemented(
      "the scalar expression ", expression.toString(), " has no Arrow compute translation"
   );
}

}  // namespace rhydb::query_engine::exec_node
