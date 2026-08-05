#pragma once

#include "silo/common/panic.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/date32_column.h"
#include "silo/storage/column/dictionary_encoded_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::storage::column {

template <typename VisitorFunction, typename... Args>
static decltype(auto) visit(schema::ColumnType type, VisitorFunction&& func, Args&&... args) {
   switch (type) {
      case schema::ColumnType::STRING:
         return func.template operator()<StringColumn>(std::forward<Args>(args)...);
      case schema::ColumnType::DICTIONARY_ENCODED:
         return func.template operator()<DictionaryEncodedColumn>(std::forward<Args>(args)...);
      case schema::ColumnType::DATE32:
         return func.template operator()<Date32Column>(std::forward<Args>(args)...);
      case schema::ColumnType::BOOL:
         return func.template operator()<BoolColumn>(std::forward<Args>(args)...);
      case schema::ColumnType::INT32:
         return func.template operator()<Int32Column>(std::forward<Args>(args)...);
      case schema::ColumnType::INT64:
         return func.template operator()<Int64Column>(std::forward<Args>(args)...);
      case schema::ColumnType::FLOAT:
         return func.template operator()<FloatColumn>(std::forward<Args>(args)...);
      case schema::ColumnType::NUCLEOTIDE_SEQUENCE:
         return func.template operator()<SequenceColumn<Nucleotide>>(std::forward<Args>(args)...);
      case schema::ColumnType::AMINO_ACID_SEQUENCE:
         return func.template operator()<SequenceColumn<AminoAcid>>(std::forward<Args>(args)...);
      case schema::ColumnType::ZSTD_COMPRESSED_STRING:
         return func.template operator()<ZstdCompressedStringColumn>(std::forward<Args>(args)...);
   }
   SILO_UNREACHABLE();
}

}  // namespace silo::storage::column