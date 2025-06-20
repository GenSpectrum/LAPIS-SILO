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
   static Database initializeDatabase(const config::InitializationFiles& initialization_files);

   static silo::schema::DatabaseSchema createSchemaFromConfigFiles(
      config::DatabaseConfig database_config,
      ReferenceGenomes reference_genomes,
      common::LineageTreeAndIdMap lineage_tree,
      common::PhyloTree phylo_tree_file
   );
};
}  // namespace silo::initialize
