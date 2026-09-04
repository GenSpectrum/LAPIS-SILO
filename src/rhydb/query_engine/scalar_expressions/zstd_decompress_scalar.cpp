#include "rhydb/query_engine/scalar_expressions/zstd_decompress_scalar.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

ZstdDecompressScalar::ZstdDecompressScalar(
   std::unique_ptr<ScalarExpression> input,
   std::string dictionary_string
)
    : input(std::move(input)),
      dictionary_string(std::move(dictionary_string)) {
   RHYDB_ASSERT(this->input != nullptr);
   RHYDB_ASSERT(!this->dictionary_string.empty());
}

std::unique_ptr<ScalarExpression> ZstdDecompressScalar::clone() const {
   return std::make_unique<ZstdDecompressScalar>(input->clone(), dictionary_string);
}

std::string ZstdDecompressScalar::toString() const {
   return fmt::format("zstd_decompress({})", input->toString());
}

std::vector<schema::ColumnIdentifier> ZstdDecompressScalar::freeIUs() const {
   return input->freeIUs();
}

std::unique_ptr<ScalarExpression> ZstdDecompressScalar::rewrite(
   const storage::Table& table,
   AmbiguityMode mode
) const {
   return std::make_unique<ZstdDecompressScalar>(input->rewrite(table, mode), dictionary_string);
}

std::unique_ptr<filter::operators::Operator> ZstdDecompressScalar::compile(
   const storage::Table& /*table*/
) const {
   // ZstdDecompressScalar is a scalar expression, not a filter predicate.
   RHYDB_UNIMPLEMENTED();
}

}  // namespace rhydb::query_engine::scalar_expressions
