#include "silo/storage/column_group.h"

#include <map>
#include <string>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo::storage {

// TODO using columns

template <>
std::map<std::string, column::IndexedStringColumn>& ColumnGroup::getColumns<
   column::IndexedStringColumn>() {
   return indexed_string_columns;
}

template <>
std::map<std::string, column::StringColumn>& ColumnGroup::getColumns<column::StringColumn>() {
   return string_columns;
}

template <>
std::map<std::string, column::Int32Column>& ColumnGroup::getColumns<column::Int32Column>() {
   return int32_columns;
}

template <>
std::map<std::string, column::Int64Column>& ColumnGroup::getColumns<column::Int64Column>() {
   return int64_columns;
}

template <>
std::map<std::string, column::BoolColumn>& ColumnGroup::getColumns<column::BoolColumn>() {
   return bool_columns;
}

template <>
std::map<std::string, column::FloatColumn>& ColumnGroup::getColumns<column::FloatColumn>() {
   return float_columns;
}

template <>
std::map<std::string, column::Date32Column>& ColumnGroup::getColumns<column::Date32Column>() {
   return date32_columns;
}

template <>
std::map<std::string, column::SequenceColumn<Nucleotide>>& ColumnGroup::getColumns<
   column::SequenceColumn<Nucleotide>>() {
   return nuc_columns;
}

template <>
std::map<std::string, column::SequenceColumn<AminoAcid>>& ColumnGroup::getColumns<
   column::SequenceColumn<AminoAcid>>() {
   return aa_columns;
}

template <>
std::map<std::string, column::ZstdCompressedStringColumn>& ColumnGroup::getColumns<
   column::ZstdCompressedStringColumn>() {
   return zstd_compressed_string_columns;
}

template <>
const std::map<std::string, column::IndexedStringColumn>& ColumnGroup::getColumns<
   column::IndexedStringColumn>() const {
   return indexed_string_columns;
}

template <>
const std::map<std::string, column::StringColumn>& ColumnGroup::getColumns<column::StringColumn>(
) const {
   return string_columns;
}

template <>
const std::map<std::string, column::Int32Column>& ColumnGroup::getColumns<column::Int32Column>() const {
   return int32_columns;
}

template <>
const std::map<std::string, column::Int64Column>& ColumnGroup::getColumns<column::Int64Column>(
) const {
   return int64_columns;
}

template <>
const std::map<std::string, column::BoolColumn>& ColumnGroup::getColumns<column::BoolColumn>(
) const {
   return bool_columns;
}

template <>
const std::map<std::string, column::FloatColumn>& ColumnGroup::getColumns<column::FloatColumn>(
) const {
   return float_columns;
}

template <>
const std::map<std::string, column::Date32Column>& ColumnGroup::getColumns<column::Date32Column>(
) const {
   return date32_columns;
}

template <>
const std::map<std::string, column::SequenceColumn<Nucleotide>>& ColumnGroup::getColumns<
   column::SequenceColumn<Nucleotide>>() const {
   return nuc_columns;
}

template <>
const std::map<std::string, column::SequenceColumn<AminoAcid>>& ColumnGroup::getColumns<
   column::SequenceColumn<AminoAcid>>() const {
   return aa_columns;
}

template <>
const std::map<std::string, column::ZstdCompressedStringColumn>& ColumnGroup::getColumns<
   column::ZstdCompressedStringColumn>() const {
   return zstd_compressed_string_columns;
}

}  // namespace silo::storage
