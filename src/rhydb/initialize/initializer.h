#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "rhydb/common/lineage_tree.h"
#include "rhydb/common/phylo_tree.h"
#include "rhydb/config/database_config.h"
#include "rhydb/config/initialize_config.h"
#include "rhydb/database.h"
#include "rhydb/storage/reference_genomes.h"

namespace rhydb::initialize {

class Initializer {
  public:
   static void createTableInDatabase(
      schema::TableName table_name,
      const config::InitializationFiles& initialization_files,
      Database& database
   );

   /// Builds the table `table_name` from `database_config` plus the `reference_columns` rows
   /// already declared for it (see `loadReferences`, which must have run first).
   static void createTableInDatabase(
      schema::TableName table_name,
      const config::DatabaseConfig& database_config,
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const common::PhyloTree& phylo_tree,
      Database& database
   );

   /// Stores the reference sequences held by the given `ReferenceGenomes` in the database's
   /// built-in `reference_genomes` table, and declares in `reference_columns` which columns of
   /// `table_name` they back: one column per reference, named after it, plus -- unless
   /// `without_unaligned_sequences` -- an `unaligned_` zstd column per nucleotide sequence, which
   /// shares that sequence's reference rather than storing a second copy of it. `table_name` is
   /// what scopes the declaration, so a second table's schema does not inherit this one's columns.
   /// Throws `schema::DuplicatePrimaryKeyException` if a reference name is already stored, whether
   /// from an earlier call or from the other sequence kind of this same `ReferenceGenomes`.
   static void loadReferences(
      const schema::TableName& table_name,
      const ReferenceGenomes& reference_genomes,
      bool without_unaligned_sequences,
      Database& database
   );

   /// Builds the schema for one table: the scalar columns from `database_config`, the sequence
   /// columns from `column_references` -- the `reference_columns` rows `loadReferences` declared
   /// for that table, which already say which columns exist, of what type, and against which
   /// reference.
   static std::shared_ptr<schema::TableSchema> createSchemaFromConfigFiles(
      const config::DatabaseConfig& database_config,
      const std::vector<ResolvedColumnReference>& column_references,
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const common::PhyloTree& phylo_tree_file
   );

   static std::optional<common::LineageTreeAndIdMap> findLineageTreeForName(
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const std::string& lineage_tree_name
   );

  private:
   static void createLineageRelationTable(
      std::string_view column_name,
      const common::LineageTreeAndIdMap& lineage_tree,
      Database& database
   );
};
}  // namespace rhydb::initialize
