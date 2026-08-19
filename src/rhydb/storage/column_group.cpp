#include "rhydb/storage/column_group.h"

#include <map>
#include <string>

#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/nucleotide_symbols.h"

namespace rhydb::storage {

using column::BoolColumn;
using column::Date32Column;
using column::DictionaryEncodedColumn;
using column::FloatColumn;
using column::Int32Column;
using column::Int64Column;
using column::SequenceColumn;
using column::StringColumn;
using column::ZstdCompressedStringColumn;

template <>
std::map<std::string, DictionaryEncodedColumn>& ColumnGroup::getColumns<DictionaryEncodedColumn>() {
   return dictionary_encoded_columns;
}

template <>
std::map<std::string, StringColumn>& ColumnGroup::getColumns<StringColumn>() {
   return string_columns;
}

template <>
std::map<std::string, Int32Column>& ColumnGroup::getColumns<Int32Column>() {
   return int32_columns;
}

template <>
std::map<std::string, Int64Column>& ColumnGroup::getColumns<Int64Column>() {
   return int64_columns;
}

template <>
std::map<std::string, BoolColumn>& ColumnGroup::getColumns<BoolColumn>() {
   return bool_columns;
}

template <>
std::map<std::string, FloatColumn>& ColumnGroup::getColumns<FloatColumn>() {
   return float_columns;
}

template <>
std::map<std::string, Date32Column>& ColumnGroup::getColumns<Date32Column>() {
   return date32_columns;
}

template <>
std::map<std::string, SequenceColumn<Nucleotide>>& ColumnGroup::getColumns<
   SequenceColumn<Nucleotide>>() {
   return nuc_columns;
}

template <>
std::map<std::string, SequenceColumn<AminoAcid>>& ColumnGroup::getColumns<
   SequenceColumn<AminoAcid>>() {
   return aa_columns;
}

template <>
std::map<std::string, ZstdCompressedStringColumn>& ColumnGroup::getColumns<
   ZstdCompressedStringColumn>() {
   return zstd_compressed_string_columns;
}

template <>
const std::map<std::string, DictionaryEncodedColumn>& ColumnGroup::getColumns<
   DictionaryEncodedColumn>() const {
   return dictionary_encoded_columns;
}

template <>
const std::map<std::string, StringColumn>& ColumnGroup::getColumns<StringColumn>() const {
   return string_columns;
}

template <>
const std::map<std::string, Int32Column>& ColumnGroup::getColumns<Int32Column>() const {
   return int32_columns;
}

template <>
const std::map<std::string, Int64Column>& ColumnGroup::getColumns<Int64Column>() const {
   return int64_columns;
}

template <>
const std::map<std::string, BoolColumn>& ColumnGroup::getColumns<BoolColumn>() const {
   return bool_columns;
}

template <>
const std::map<std::string, FloatColumn>& ColumnGroup::getColumns<FloatColumn>() const {
   return float_columns;
}

template <>
const std::map<std::string, Date32Column>& ColumnGroup::getColumns<Date32Column>() const {
   return date32_columns;
}

template <>
const std::map<std::string, SequenceColumn<Nucleotide>>& ColumnGroup::getColumns<
   SequenceColumn<Nucleotide>>() const {
   return nuc_columns;
}

template <>
const std::map<std::string, SequenceColumn<AminoAcid>>& ColumnGroup::getColumns<
   SequenceColumn<AminoAcid>>() const {
   return aa_columns;
}

template <>
const std::map<std::string, ZstdCompressedStringColumn>& ColumnGroup::getColumns<
   ZstdCompressedStringColumn>() const {
   return zstd_compressed_string_columns;
}

}  // namespace rhydb::storage
