#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

/// A scalar expression that zstd-decompresses a single sequence-typed column into the
/// STRING value it encodes. The output is a STRING-typed column with the same name as
/// the input. Used to expand the compressed columns of a table scan on demand.
///
/// The dictionary used for decompression depends on the input column type:
///  - SequenceColumn (nucleotide / amino acid): the reference genome serves as dictionary.
///  - ZstdCompressedStringColumn: the column's own trained zstd dictionary.
/// In both cases it is passed through `dictionary_string` and must be non-empty.
class ZstdDecompressScalar : public Expression {
  public:
   /// The compressed input column produced by the child node.
   schema::ColumnIdentifier input_column;

   /// The zstd dictionary used for decompression (a reference genome for sequence columns,
   /// or a trained dictionary for zstd-compressed-string columns). Must be non-empty.
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
