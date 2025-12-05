#pragma once

#include "silo/common/lineage_tree.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/database_config.h"
#include "silo/config/initialize_config.h"
#include "silo/database.h"
#include "silo/storage/reference_genomes.h"

namespace silo::initialize {

class Initializer {
  public:
   static void createTableInDatabase(
      schema::TableName table_name,
      const config::InitializationFiles& initialization_files,
      Database& database
   );

   static silo::schema::TableSchema createSchemaFromConfigFiles(
      config::DatabaseConfig database_config,
      ReferenceGenomes reference_genomes,
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const common::PhyloTree& phylo_tree_file,
      bool without_unaligned_sequences
   );

   static std::optional<common::LineageTreeAndIdMap> findLineageTreeForName(
      const std::map<std::filesystem::path, common::LineageTreeAndIdMap>& lineage_trees,
      const std::string& lineage_tree_name
   );
};
}  // namespace silo::initialize
