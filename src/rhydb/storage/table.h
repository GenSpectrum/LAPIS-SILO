#pragma once

#include <expected>
#include <filesystem>
#include <map>
#include <string>

#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/column/row_layout.h"
#include "rhydb/storage/column_group.h"

namespace rhydb::storage {

class ColumnGroupBuilder;
class Table;

/// The tables describing one lineage system: the relation table holding its parent->child edges,
/// and the alias table mapping alternative names onto canonical lineages (absent when the
/// definition declares none). Both are owned by the Database; a definition outlives every query
/// that reads it.
struct LineageDefinition {
   const Table* relation = nullptr;
   const Table* aliases = nullptr;
};

class Table {
  public:
   schema::TableName table_name;
   std::shared_ptr<schema::TableSchema> schema;
   ColumnGroup columns;
   uint32_t sequence_count = 0;
   /// The lineage definitions a query over this table can resolve `lineage(...)` against, by the
   /// name the query uses for them (the relation table's name). Kept here so that a filter, which
   /// only ever sees the table it filters, can reach the tree describing one of its columns.
   /// Linked by the owning Database whenever a table is created or loaded; not serialized.
   std::map<std::string, LineageDefinition> lineage_definitions;
   /// The shared per-chunk row layout of this table partition: every column is appended to in
   /// lockstep, so this single layout is the source of truth for iterating the partition's rows by
   /// `RowId`. `sequence_count == row_layout.numRows()`.
   column::RowLayout row_layout;

   explicit Table(schema::TableName table_name, std::shared_ptr<schema::TableSchema> schema);

   Table(Table&& other) = default;
   Table& operator=(Table&& other) = default;

   Table(const Table& other) = delete;
   Table& operator=(const Table& other) = delete;

   template <class Archive>
   void serializeData(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & columns;
      archive & sequence_count;
      archive & row_layout;
      // clang-format on
   }

   [[nodiscard]] nlohmann::json logTable() const;

   void validate() const;

   /// Apply a finalized ingestion chunk (one buffer per column) to the columns'
   /// global structures. Consumes (clears) the builder's buffers.
   std::expected<void, std::string> bulkInsert(ColumnGroupBuilder& block);

   void finalize();

   void loadData(const std::filesystem::path& path);
   void saveData(const std::filesystem::path& path);
   void validatePrimaryKeyUnique() const;

  private:
   void validateNucleotideSequences() const;
   void validateAminoAcidSequences() const;
   void validateMetadataColumns() const;

   template <typename Column>
   void validateColumnsHaveSize(
      const std::map<std::string, Column>& columnsOfTheType,
      const std::string& columnType
   ) const;
};

}  // namespace rhydb::storage
