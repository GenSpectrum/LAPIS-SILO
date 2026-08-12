#pragma once

#include <filesystem>

#include "rhydb/append/table_inserter.h"
#include "rhydb/common/data_version.h"
#include "rhydb/common/silo_directory.h"
#include "rhydb/database_info.h"
#include "rhydb/query_engine/query_plan.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb {

/// Describes a single column for the generic `createTableFromColumns` API.
/// `type` is a lowercase type name (e.g. "string", "indexed_string", "date", "bool", "int",
/// "float", "nucleotide_sequence", "amino_acid_sequence", "zstd_compressed_string"). Columns that
/// need a reference (the two sequence types and "zstd_compressed_string") take it from the built-in
/// `_references` table rather than from this struct (see `createTableFromColumns`).
struct ColumnDefinition {
   std::string name;
   std::string type;
};

class Database {
  public:
   /// Name of the built-in table holding reference sequences for sequence / zstd-compressed
   /// columns. Every Database has it automatically (see `createReferencesTable`); it is populated
   /// by the caller and read by `createTableFromColumns`. The leading underscore marks it as
   /// internal and hides it from `getTables`.
   static constexpr std::string_view REFERENCES_TABLE_NAME = "_references";

   schema::DatabaseSchema schema;
   std::map<schema::TableName, std::shared_ptr<storage::Table>> tables;

   void updateDataVersion();

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database();

   explicit Database(schema::DatabaseSchema database_schema);

   virtual ~Database() = default;

   void createTable(
      schema::TableName table_name,
      std::shared_ptr<schema::TableSchema> table_schema
   );

   void appendData(
      const schema::TableName& table_name,
      std::istream& input_stream,
      append::ClusteredBufferingOptions clustering_options = {}
   );

   /// Generic table creation: builds a table whose columns can be of any supported type. The first
   /// entry of `columns` becomes the table's primary key, so `columns` must be non-empty and its
   /// first entry must be of type "string". Every entry describes one column via its `type` name.
   /// Columns that need a reference (the two sequence types and "zstd_compressed_string") take it
   /// from the built-in `_references` table (STRING columns `name` and `reference`), which the
   /// caller must have populated beforehand: its entry whose `name` equals the column name supplies
   /// that column's reference. Throws if `columns` is empty, its first entry is not a string, a
   /// type name is unknown, a required reference is missing/invalid, or a column name is
   /// duplicated.
   void createTableFromColumns(
      const std::string& table_name,
      const std::vector<ColumnDefinition>& columns
   );

   void appendDataFromFile(const std::string& table_name, const std::string& file_path);

   void appendDataFromString(const std::string& table_name, std::string json_string);

   void printAllData(const std::string& table_name) const;

   std::string getNucleotideReferenceSequence(
      const std::string& table_name,
      const std::string& sequence_name
   );

   std::string getAminoAcidReferenceSequence(
      const std::string& table_name,
      const std::string& sequence_name
   );

   roaring::Roaring getFilteredBitmap(const std::string& table_name, const std::string& filter);

   /// Assigns the scalar `value` to the column `column_name` of `table_name` for every row matched
   /// by the SaneQL `filter_expression`. `value` is a single SaneQL literal (parsed by the same
   /// lexer/parser as queries) matching the column's type, e.g. `3`, `3.14`, `true`, or
   /// `'2021-03-15'::date`; the literal `null` clears the matched rows. Only scalar value columns
   /// (INT32, FLOAT, DATE32, BOOL) can be updated; other column types raise an error.
   void updateColumn(
      const std::string& table_name,
      const std::string& column_name,
      const std::string& value,
      const std::string& filter_expression
   );

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static std::optional<Database> loadDatabaseStateFromPath(
      const std::filesystem::path& save_directory
   );

   static Database loadDatabaseState(const RhyDBDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DataVersion::Timestamp getDataVersionTimestamp() const;

   [[nodiscard]] std::string executeQueryAsArrowIpc(const std::string& query_string) const;

   [[nodiscard]] std::string getTablesAsArrowIpc() const;

  private:
   [[nodiscard]] arrow::Result<std::string> getTablesAsArrowIpcImpl() const;

   /// Creates the built-in `_references` table (string columns `name` and `reference`, with `name`
   /// as the primary key). Called from every constructor so the table always exists.
   void createReferencesTable();

   /// Looks up the reference string for `column_name` in the built-in `_references` table (see
   /// `createTableFromColumns`). Throws if the table is malformed or has no matching entry.
   std::string lookupReferenceForColumn(const std::string& column_name);
};

}  // namespace rhydb
