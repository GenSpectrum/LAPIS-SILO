#include "rhydb/schema/data_type.h"

#include <initializer_list>
#include <optional>
#include <string_view>

#include "rhydb/common/panic.h"

namespace rhydb::schema {

std::string_view toSchemaTypeName(const DataType& type) {
   switch (type.getKind()) {
      case DataType::Kind::STRING:
         return "string";
      case DataType::Kind::DATE32:
         return "date";
      case DataType::Kind::BOOL:
         return "boolean";
      case DataType::Kind::INT32:
         return "int32";
      case DataType::Kind::INT64:
         return "int64";
      case DataType::Kind::FLOAT:
         return "float";
      case DataType::Kind::NUCLEOTIDE_SEQUENCE:
         return "nucleotideSequence";
      case DataType::Kind::AMINO_ACID_SEQUENCE:
         return "aminoAcidSequence";
      case DataType::Kind::ZSTD_COMPRESSED_STRING:
         return "zstdCompressedString";
      case DataType::Kind::DICTIONARY_ENCODED:
         break;
   }
   RHYDB_UNREACHABLE();
}

std::optional<DataType> fromSchemaTypeName(std::string_view name) {
   if (name == "int") {
      return DataType::INT32;
   }
   for (const DataType& type : {
           DataType::STRING,
           DataType::DATE32,
           DataType::BOOL,
           DataType::INT32,
           DataType::INT64,
           DataType::FLOAT,
           DataType::NUCLEOTIDE_SEQUENCE,
           DataType::AMINO_ACID_SEQUENCE,
           DataType::ZSTD_COMPRESSED_STRING,
        }) {
      if (toSchemaTypeName(type) == name) {
         return type;
      }
   }
   return std::nullopt;
}

}  // namespace rhydb::schema
