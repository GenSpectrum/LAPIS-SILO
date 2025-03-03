#pragma once

#include <optional>

#include "silo/common/lineage_tree.h"
#include "silo/common/table_reader.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/preprocessing/validated_ndjson_file.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::preprocessing {

class Initializer {
   config::PreprocessingConfig preprocessing_config;
   config::DatabaseConfig database_config;
   ReferenceGenomes reference_genomes;
   common::LineageTreeAndIdMap lineage_tree;

  public:
   Initializer(
      config::PreprocessingConfig preprocessing_config_,
      config::DatabaseConfig database_config_,
      ReferenceGenomes reference_genomes_,
      common::LineageTreeAndIdMap lineage_tree_
   );

   Database initialize();

  private:
   void finalizeConfig();
   void validateConfig();

   Database buildDatabase();
};
}  // namespace silo::preprocessing
