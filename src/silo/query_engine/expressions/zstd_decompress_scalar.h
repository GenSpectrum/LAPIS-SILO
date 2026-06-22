#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

/// A scalar expression that decompresses a single zstd-compressed (or delta-encoded
/// sequence) column using the given reference/dictionary string.  The input column
/// must be one of the sequence-typed column types; the output is a STRING-typed
/// column with the same name.
class ZstdDecompressScalar : public Expression {
  public:
   /// The compressed input column produced by the child node.
   schema::ColumnIdentifier input_column;

   /// The reference sequence or zstd dictionary used for decompression.
   std::string dictionary_string;

   ZstdDecompressScalar(schema::ColumnIdentifier input_column, std::string dictionary_string);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::STRING; }

   static constexpr Kind KIND = Kind::ZSTD_DECOMPRESS_SCALAR;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> clone() const override;

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
