#pragma once

#include <memory>
#include <vector>

#include <arrow/type.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::query_engine::exec_node {

std::shared_ptr<arrow::DataType> columnTypeToArrowType(schema::ColumnType column_type);

std::shared_ptr<arrow::Schema> columnsToArrowSchema(
   const std::vector<silo::schema::ColumnIdentifier>& columns
);

template <storage::column::Column Column>
struct ArrowBuilderSelector;

template <>
struct ArrowBuilderSelector<storage::column::StringColumnPartition> {
   using builder_type = arrow::StringBuilder;
   using value_type = std::string;
};

template <>
struct ArrowBuilderSelector<storage::column::IndexedStringColumnPartition> {
   using builder_type = arrow::StringBuilder;
   using value_type = std::string;
};

template <>
struct ArrowBuilderSelector<storage::column::SequenceColumnPartition<Nucleotide>> {
   using builder_type = arrow::BinaryBuilder;
   using value_type = std::string;
};

template <>
struct ArrowBuilderSelector<storage::column::SequenceColumnPartition<AminoAcid>> {
   using builder_type = arrow::BinaryBuilder;
   using value_type = std::string;
};

template <>
struct ArrowBuilderSelector<storage::column::ZstdCompressedStringColumnPartition> {
   using builder_type = arrow::BinaryBuilder;
   using value_type = std::string;
};

template <>
struct ArrowBuilderSelector<storage::column::FloatColumnPartition> {
   using builder_type = arrow::DoubleBuilder;
   using value_type = double;
};

template <>
struct ArrowBuilderSelector<storage::column::BoolColumnPartition> {
   using builder_type = arrow::BooleanBuilder;
   using value_type = bool;
};

template <>
struct ArrowBuilderSelector<storage::column::IntColumnPartition> {
   using builder_type = arrow::Int32Builder;
   using value_type = int32_t;
};

template <>
struct ArrowBuilderSelector<storage::column::DateColumnPartition> {
   using builder_type = arrow::StringBuilder;
   using value_type = std::string;
};

template <storage::column::Column ColumnType>
using ArrowBuilder = typename ArrowBuilderSelector<ColumnType>::builder_type;

}  // namespace silo::query_engine::exec_node