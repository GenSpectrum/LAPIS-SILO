#pragma once

#include <optional>

#include "silo/common/lineage_tree.h"
#include "silo/common/table_reader.h"
#include "silo/config/database_config.h"
#include "silo/config/initialize_config.h"
#include "silo/database.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::initialize {

class Initializer {
  public:
   static Database initializeDatabase(config::InitializationFiles initialization_files);

   static silo::schema::DatabaseSchema createSchemaFromConfigFiles(
      config::DatabaseConfig database_config,
      ReferenceGenomes reference_genomes,
      common::LineageTreeAndIdMap lineage_tree
   );
};
}  // namespace silo::initialize
