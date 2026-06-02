#pragma once

#include <expected>
#include <map>
#include <string>
#include <vector>

#include <simdjson.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date32_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::storage {

class ColumnGroup;

/// Accumulates one ingestion chunk (at most column::COLUMN_CHUNK_SIZE rows) by
/// buffering the extracted value of each row into a per-column builder. The
/// finalized chunks are handed to Table::bulkInsert, which applies them to the
/// columns' global structures.
class ColumnGroupBuilder {
  public:
   std::vector<schema::ColumnIdentifier> metadata;

   std::map<std::string, column::StringColumn::Builder> string_column_builders;
   std::map<std::string, column::IndexedStringColumn::Builder> indexed_string_column_builders;
   std::map<std::string, column::BoolColumn::Builder> bool_column_builders;
   std::map<std::string, column::IntColumn::Builder> int_column_builders;
   std::map<std::string, column::FloatColumn::Builder> float_column_builders;
   std::map<std::string, column::Date32Column::Builder> date32_column_builders;
   std::map<std::string, column::SequenceColumn<Nucleotide>::Builder> nuc_column_builders;
   std::map<std::string, column::SequenceColumn<AminoAcid>::Builder> aa_column_builders;
   std::map<std::string, column::ZstdCompressedStringColumn::Builder>
      zstd_compressed_string_column_builders;

   /// The sequence column builders are seeded with each column's current
   /// (possibly adapted) local reference, so sequences buffered into a chunk are
   /// diffed against the same reference basis as the already-stored rows.
   ColumnGroupBuilder(const schema::TableSchema& schema, const ColumnGroup& columns);

   /// Extract the value of one column for the current row from the json line and
   /// buffer it into the matching column builder.
   std::expected<void, std::string> addJsonValueToColumn(
      const schema::ColumnIdentifier& column_identifier,
      simdjson::ondemand::value& value
   );

   /// Number of rows buffered into the current chunk.
   [[nodiscard]] size_t numBufferedRows() const;

   template <column::Column ColumnType>
   std::map<std::string, typename ColumnType::Builder>& getColumnBuilders();
};

template <>
std::map<std::string, column::StringColumn::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::StringColumn>();
template <>
std::map<std::string, column::IndexedStringColumn::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::IndexedStringColumn>();
template <>
std::map<std::string, column::BoolColumn::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::BoolColumn>();
template <>
std::map<std::string, column::IntColumn::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::IntColumn>();
template <>
std::map<std::string, column::FloatColumn::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::FloatColumn>();
template <>
std::map<std::string, column::Date32Column::Builder>& ColumnGroupBuilder::getColumnBuilders<
   column::Date32Column>();
template <>
std::map<std::string, column::SequenceColumn<Nucleotide>::Builder>& ColumnGroupBuilder::
   getColumnBuilders<column::SequenceColumn<Nucleotide>>();
template <>
std::map<std::string, column::SequenceColumn<AminoAcid>::Builder>& ColumnGroupBuilder::
   getColumnBuilders<column::SequenceColumn<AminoAcid>>();
template <>
std::map<std::string, column::ZstdCompressedStringColumn::Builder>& ColumnGroupBuilder::
   getColumnBuilders<column::ZstdCompressedStringColumn>();

}  // namespace silo::storage
