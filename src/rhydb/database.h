#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "rhydb/append/table_inserter.h"
#include "rhydb/common/data_version.h"
#include "rhydb/common/silo_directory.h"
#include "rhydb/database_info.h"
#include "rhydb/query_engine/query_plan.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb {

/// Describes a single column for the generic `createTableFromColumns` API.
struct ColumnDefinition {
   /// Name of the column, unique within the table.
   std::string name;

   /// Lowercase type name, one of "string", "indexed_string", "date", "bool", "int", "float",
   /// "nucleotide_sequence", "amino_acid_sequence", "zstd_compressed_string". A column of one of
   /// the three types that need a reference must have its `reference_columns` row already in place;
   /// which reference backs it is stated there, not here.
   std::string type;
};

/// One row of the built-in `reference_genomes` table: a reference sequence under its own name,
/// which no longer has to match the name of any column. `type` is the kind of the reference itself
/// ("nucleotide_sequence" or "amino_acid_sequence"), used to check it against the type of each
/// column that claims it (see `addColumnReferences`); the generic path may leave it empty.
struct ReferenceEntry {
   std::string name;
   std::string reference;
   std::string type;
};

/// One row of the built-in `reference_columns` table: the statement that a given column of a given
/// table takes its reference from a given `reference_genomes` entry. This is the mapping that used
/// to be implicit in a column and its reference sharing a name.
struct ColumnReferenceEntry {
   std::string table_name;
   std::string column_name;
   /// Lowercase type name of the column, one of "nucleotide_sequence", "amino_acid_sequence" or
   /// "zstd_compressed_string".
   std::string column_type;
   std::string reference_name;
};

/// A `reference_columns` row with its reference resolved from the `reference_genomes` store: what a
/// schema builder needs to turn the mapping into a column.
struct ResolvedColumnReference {
   std::string column_name;
   schema::ColumnType column_type;
   /// The reference string, from the `reference_genomes` entry named by `reference_name`.
   std::string reference;
   std::string reference_name;
};

class Database {
  public:
   /// Name of the built-in table holding reference sequences for sequence / zstd-compressed
   /// columns (STRING columns `name`, `reference`, `type`, keyed on `name`). Every Database has it:
   /// a new one gets it from `createReferenceGenomesTable`, and a database loaded from disk gets it
   /// from its persisted schema (the serialization version was bumped when the table was
   /// introduced, so snapshots without it are rejected as incompatible). It is populated by the
   /// caller and read by `createTableFromColumns` and by the preprocessing/initialize path (see
   /// `getReferences`). It is a normal, queryable table and is listed by `getTables`.
   static constexpr std::string_view REFERENCE_GENOMES_TABLE_NAME = "reference_genomes";

   /// Name of the built-in table mapping a table's columns to the `reference_genomes` entries
   /// backing them (STRING columns `table_name`, `column_name`, `column_type`, `reference_name`).
   /// A row is identified by its `table_name` and `column_name` together, which
   /// `TableSchema::primary_key` cannot express, so the table declares no primary key. Like
   /// `reference_genomes` it is present in every Database and is a normal, queryable table.
   static constexpr std::string_view REFERENCE_COLUMNS_TABLE_NAME = "reference_columns";

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
   /// from the `reference_columns` row already declared for them: this reads that mapping and never
   /// writes it, so the caller states which reference backs which column -- by
   /// `addColumnReferences` or by appending to `reference_columns` directly -- before creating the
   /// table. Two columns may name one reference. Throws if `columns` is empty, its first entry is
   /// not a string, a type name is unknown, a column name is duplicated, or a column needing a
   /// reference has no declaration for it or one that disagrees about its type.
   void createTableFromColumns(
      const std::string& table_name,
      const std::vector<ColumnDefinition>& columns
   );

   void appendDataFromFile(const std::string& table_name, const std::string& file_path);

   void appendDataFromString(const std::string& table_name, std::string json_string);

   void printAllData(const std::string& table_name) const;

   /// Reads every row of the built-in `reference_genomes` table (see
   /// `REFERENCE_GENOMES_TABLE_NAME`) and returns them as `ReferenceEntry` records. Used by the
   /// preprocessing/initialize path to source the reference sequences (and their
   /// nucleotide/amino-acid typing) when building a table schema.
   std::vector<ReferenceEntry> getReferences();

   /// Appends `entries` to the built-in `reference_genomes` table as one batch, the only supported
   /// way to add references. Reference names must be unique across the whole table, including
   /// across the nucleotide/amino-acid `type`: the `type` column only carries that distinction
   /// through to `Initializer::createSchemaFromConfigFiles` (the table itself is type-less), it
   /// does not make `(name, type)` a key. Everything downstream resolves by name alone --
   /// a schema holding two same-named columns cannot be instantiated at all, because
   /// `TableSchema::getColumnMetadata` looks up by name and `storage::Table`'s constructor
   /// unwraps the result. Throws `schema::DuplicatePrimaryKeyException` (`name` being this table's
   /// primary key) if a name is repeated within `entries` or already present, appending nothing.
   void addReferences(const std::vector<ReferenceEntry>& entries);

   /// Appends `entries` to the built-in `reference_columns` table as one batch, declaring which
   /// `reference_genomes` entry backs each named column of each named table. Validates that every
   /// `reference_name` exists in the store, that no `(table_name, column_name)` pair is declared
   /// twice, and that a stated reference `type` is usable for the column type claiming it -- a
   /// nucleotide reference backs a "nucleotide_sequence" or "zstd_compressed_string" column, an
   /// amino acid reference an "amino_acid_sequence" one. Throws `std::runtime_error` on any of
   /// those, appending nothing.
   void addColumnReferences(const std::vector<ColumnReferenceEntry>& entries);

   /// The `reference_columns` rows declared for `table_name`, each with its reference resolved from
   /// the `reference_genomes` store. This is how a schema builder learns a table's sequence columns
   /// -- scoped to that table, so a second table does not inherit the first one's columns.
   std::vector<ResolvedColumnReference> getColumnReferences(const std::string& table_name);

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

   /// Creates the built-in `reference_genomes` table (string columns `name`, `reference` and
   /// `type`, with `name` as the primary key). Called from the default constructor; the
   /// schema-taking constructor instead materializes the table from the persisted schema it is
   /// handed (see `REFERENCE_GENOMES_TABLE_NAME`).
   void createReferenceGenomesTable();

   /// Creates the built-in `reference_columns` table (see `REFERENCE_COLUMNS_TABLE_NAME`). Called
   /// from the default constructor alongside `createReferenceGenomesTable`; a database loaded from
   /// disk materializes it from its persisted schema instead.
   void createReferenceColumnsTable();

   /// The `(table_name, column_name)` pair of every `reference_columns` row -- the natural key of
   /// the mapping -- used to reject a column whose reference is already declared.
   [[nodiscard]] std::set<std::pair<std::string, std::string>> declaredColumnReferences();

   /// Looks up the `reference_genomes` entry named `reference_name`. Throws if the table is
   /// malformed or holds no entry of that name. Unlike the mapping this used to do by itself, it is
   /// now a plain lookup by the reference's own name: which column uses it is stated in
   /// `reference_columns`.
   [[nodiscard]] ReferenceEntry lookupReference(const std::string& reference_name);
};

}  // namespace rhydb
