#pragma once

#include <memory>
#include <vector>

#include <arrow/type.h>

#include "silo/schema/database_schema.h"

namespace silo::query_engine::exec_node {

const std::shared_ptr<arrow::DataType> columnTypeToArrowType(schema::ColumnType column_type);

std::shared_ptr<arrow::Schema> columnsToArrowSchema(
   const std::vector<silo::schema::ColumnIdentifier>& columns
);

template <schema::ColumnType T>
struct ArrowBuilderSelector;

template <>
struct ArrowBuilderSelector<schema::ColumnType::STRING> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::INDEXED_STRING> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::NUCLEOTIDE_SEQUENCE> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::AMINO_ACID_SEQUENCE> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::ZSTD_COMPRESSED_STRING> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::FLOAT> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::BOOL> {
   using type = arrow::StringBuilder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::INT> {
   using type = arrow::Int32Builder;
};

template <>
struct ArrowBuilderSelector<schema::ColumnType::DATE> {
   using type = arrow::Int32Builder;
};

template <storage::column::Column ColumnType>
class ArrowBuilder {
  public:
   using BuilderType = typename ArrowBuilderSelector<ColumnType::TYPE>::type;

   ArrowBuilder()
       : builder(std::make_unique<BuilderType>()) {}

   BuilderType* get() { return builder.get(); }

  private:
   std::unique_ptr<BuilderType> builder;
};

}  // namespace silo::query_engine::exec_node