#include "silo/query_engine/expressions/zstd_decompress_scalar.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

ZstdDecompressScalar::ZstdDecompressScalar(
   schema::ColumnIdentifier input_column,
   std::string dictionary_string
)
    : input_column(std::move(input_column)),
      dictionary_string(std::move(dictionary_string)) {}

std::unique_ptr<Expression> ZstdDecompressScalar::clone() const {
   return std::make_unique<ZstdDecompressScalar>(input_column, dictionary_string);
}

std::string ZstdDecompressScalar::toString() const {
   return fmt::format("zstd_decompress({})", input_column.name);
}

std::vector<schema::ColumnIdentifier> ZstdDecompressScalar::freeIUs() const {
   return {input_column};
}

std::unique_ptr<Expression> ZstdDecompressScalar::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return clone();
}

std::unique_ptr<filter::operators::Operator> ZstdDecompressScalar::compile(
   const storage::Table& /*table*/
) const {
   // ZstdDecompressScalar is a scalar expression, not a filter predicate.
   SILO_UNIMPLEMENTED();
}

}  // namespace silo::query_engine::expressions
