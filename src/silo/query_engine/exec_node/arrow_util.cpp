#include "silo/query_engine/exec_node/arrow_util.h"

namespace silo::query_engine::exec_node {

std::shared_ptr<arrow::DataType> columnTypeToArrowType(schema::ColumnType column_type) {
   switch (column_type) {
      case schema::ColumnType::STRING:
      case schema::ColumnType::INDEXED_STRING:
      case schema::ColumnType::DATE:
         return arrow::utf8();
      case schema::ColumnType::BOOL:
         return arrow::boolean();
      case schema::ColumnType::INT32:
         return arrow::int32();
      case schema::ColumnType::INT64:
         return arrow::int64();
      case schema::ColumnType::FLOAT:
         return arrow::float64();
      case schema::ColumnType::AMINO_ACID_SEQUENCE:
      case schema::ColumnType::NUCLEOTIDE_SEQUENCE:
      case schema::ColumnType::ZSTD_COMPRESSED_STRING:
         return arrow::binary();
   }
   SILO_UNREACHABLE();
}

std::shared_ptr<arrow::Schema> columnsToArrowSchema(
   const std::vector<silo::schema::ColumnIdentifier>& columns
) {
   std::vector<std::shared_ptr<arrow::Field>> fields;
   for (const auto& [name, type] : columns) {
      auto arrow_type = columnTypeToArrowType(type);
      fields.emplace_back(std::make_shared<arrow::Field>(name, arrow_type));
   }
   return std::make_shared<arrow::Schema>(fields);
}

}  // namespace silo::query_engine::exec_node
