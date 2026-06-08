#include "silo/query_engine/operators/zstd_decompress_node.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/true.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"
#include "silo/storage/table.h"

using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
using silo::schema::TableSchema;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::StringColumnMetadata;
using silo::storage::column::ZstdCompressedStringColumnMetadata;
namespace operators = silo::query_engine::operators;

namespace {

ColumnIdentifier col(std::string name, ColumnType type) {
   return ColumnIdentifier{.name = std::move(name), .type = type};
}

ColumnIdentifier stringCol(std::string name) {
   return col(std::move(name), ColumnType::STRING);
}

// Builds a TableSchema that contains a string primary key plus the given column.
std::shared_ptr<TableSchema> schemaWithColumn(
   const ColumnIdentifier& column,
   std::shared_ptr<ColumnMetadata> metadata
) {
   ColumnIdentifier primary_key = stringCol("id");
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)},
      {column, std::move(metadata)}
   };
   return std::make_shared<TableSchema>(std::move(col_meta), primary_key);
}

std::shared_ptr<silo::storage::Table> dummyTable() {
   ColumnIdentifier primary_key = stringCol("id");
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> col_meta{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)}
   };
   auto schema = std::make_shared<TableSchema>(std::move(col_meta), primary_key);
   return std::make_shared<silo::storage::Table>(silo::schema::TableName::getDefault(), schema);
}

// The table identity is irrelevant for the schema-only assertions below; only the field
// list matters because ZstdDecompressNode reads it through child->getOutputSchema().
operators::QueryNodePtr makeScan(std::vector<ColumnIdentifier> fields) {
   return std::make_unique<operators::TableScanNode>(
      dummyTable(), std::make_unique<silo::query_engine::expressions::True>(), std::move(fields)
   );
}

// --- buildDecompressColumnMapping ---

TEST(BuildDecompressColumnMapping, decodesNucleotideReferenceAndMapsToString) {
   auto nuc_col = col("nuc", ColumnType::NUCLEOTIDE_SEQUENCE);
   using N = silo::Nucleotide::Symbol;
   auto metadata = std::make_shared<SequenceColumnMetadata<silo::Nucleotide>>(
      "nuc", std::vector<N>{N::A, N::C, N::G, N::T}
   );
   const std::map<ColumnIdentifier, std::shared_ptr<TableSchema>> table_schemas{
      {nuc_col, schemaWithColumn(nuc_col, metadata)}
   };

   auto mapping = operators::buildDecompressColumnMapping({nuc_col}, table_schemas);

   ASSERT_EQ(mapping.size(), 1);
   EXPECT_EQ(mapping[0].input, nuc_col);
   EXPECT_EQ(mapping[0].output, stringCol("nuc"));
   EXPECT_EQ(mapping[0].reference, "ACGT");
}

TEST(BuildDecompressColumnMapping, decodesAminoAcidReference) {
   auto aa_col = col("aa", ColumnType::AMINO_ACID_SEQUENCE);
   using A = silo::AminoAcid::Symbol;
   auto metadata =
      std::make_shared<SequenceColumnMetadata<silo::AminoAcid>>("aa", std::vector<A>{A::M, A::A});
   const std::map<ColumnIdentifier, std::shared_ptr<TableSchema>> table_schemas{
      {aa_col, schemaWithColumn(aa_col, metadata)}
   };

   auto mapping = operators::buildDecompressColumnMapping({aa_col}, table_schemas);

   ASSERT_EQ(mapping.size(), 1);
   EXPECT_EQ(mapping[0].output, stringCol("aa"));
   EXPECT_EQ(mapping[0].reference, "MA");
}

TEST(BuildDecompressColumnMapping, usesDictionaryStringForZstdColumn) {
   auto zstd_col = col("seq", ColumnType::ZSTD_COMPRESSED_STRING);
   auto metadata = std::make_shared<ZstdCompressedStringColumnMetadata>("seq", "the-dictionary");
   const std::map<ColumnIdentifier, std::shared_ptr<TableSchema>> table_schemas{
      {zstd_col, schemaWithColumn(zstd_col, metadata)}
   };

   auto mapping = operators::buildDecompressColumnMapping({zstd_col}, table_schemas);

   ASSERT_EQ(mapping.size(), 1);
   EXPECT_EQ(mapping[0].output, stringCol("seq"));
   EXPECT_EQ(mapping[0].reference, "the-dictionary");
}

TEST(BuildDecompressColumnMapping, skipsPassThroughColumnsNotInTableSchemas) {
   auto nuc_col = col("nuc", ColumnType::NUCLEOTIDE_SEQUENCE);
   auto id_col = stringCol("id");
   auto date_col = col("date", ColumnType::DATE32);
   using N = silo::Nucleotide::Symbol;
   auto metadata =
      std::make_shared<SequenceColumnMetadata<silo::Nucleotide>>("nuc", std::vector<N>{N::A});
   const std::map<ColumnIdentifier, std::shared_ptr<TableSchema>> table_schemas{
      {nuc_col, schemaWithColumn(nuc_col, metadata)}
   };

   // Pass-through columns (id, date) appear in the child schema but not in table_schemas.
   auto mapping =
      operators::buildDecompressColumnMapping({id_col, nuc_col, date_col}, table_schemas);

   ASSERT_EQ(mapping.size(), 1);
   EXPECT_EQ(mapping[0].input, nuc_col);
}

TEST(BuildDecompressColumnMapping, returnsEmptyWhenNoColumnNeedsDecompression) {
   auto mapping = operators::buildDecompressColumnMapping(
      {stringCol("id"), col("date", ColumnType::DATE32)}, {}
   );

   EXPECT_THAT(mapping, ::testing::IsEmpty());
}

TEST(BuildDecompressColumnMapping, preservesChildSchemaOrderForMultipleColumns) {
   auto nuc_col = col("nuc", ColumnType::NUCLEOTIDE_SEQUENCE);
   auto aa_col = col("aa", ColumnType::AMINO_ACID_SEQUENCE);
   using N = silo::Nucleotide::Symbol;
   using A = silo::AminoAcid::Symbol;
   auto nuc_meta =
      std::make_shared<SequenceColumnMetadata<silo::Nucleotide>>("nuc", std::vector<N>{N::A});
   auto aa_meta =
      std::make_shared<SequenceColumnMetadata<silo::AminoAcid>>("aa", std::vector<A>{A::M});
   const std::map<ColumnIdentifier, std::shared_ptr<TableSchema>> table_schemas{
      {nuc_col, schemaWithColumn(nuc_col, nuc_meta)}, {aa_col, schemaWithColumn(aa_col, aa_meta)}
   };

   // Child exposes aa before nuc; the mapping must follow that order, not the map's order.
   auto mapping =
      operators::buildDecompressColumnMapping({stringCol("id"), aa_col, nuc_col}, table_schemas);

   ASSERT_EQ(mapping.size(), 2);
   EXPECT_EQ(mapping[0].input, aa_col);
   EXPECT_EQ(mapping[1].input, nuc_col);
}

// --- ZstdDecompressNode::getOutputSchema ---

TEST(ZstdDecompressNodeGetOutputSchema, replacesMappedColumnsWithStringAndKeepsOrder) {
   auto nuc_col = col("nuc", ColumnType::NUCLEOTIDE_SEQUENCE);
   auto id_col = stringCol("id");
   auto date_col = col("date", ColumnType::DATE32);

   std::vector<operators::ZstdDecompressNode::ColumnMapping> mapping{
      {.input = nuc_col, .output = stringCol("nuc"), .reference = "ACGT"}
   };
   const operators::ZstdDecompressNode node{
      makeScan({id_col, nuc_col, date_col}), std::move(mapping)
   };

   EXPECT_THAT(node.getOutputSchema(), ::testing::ElementsAre(id_col, stringCol("nuc"), date_col));
}

TEST(ZstdDecompressNodeGetOutputSchema, isIdentityWhenMappingEmpty) {
   auto id_col = stringCol("id");
   auto date_col = col("date", ColumnType::DATE32);
   const operators::ZstdDecompressNode node{makeScan({id_col, date_col}), {}};

   EXPECT_THAT(node.getOutputSchema(), ::testing::ElementsAre(id_col, date_col));
}

}  // namespace
