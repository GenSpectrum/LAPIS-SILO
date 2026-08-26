#pragma once

#include "rhydb/append/bam/bam_ingest_options.h"
#include "rhydb/config/preprocessing_config.h"

#include "rhydb/database.h"

namespace rhydb::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config);

/// Like preprocessing(), but the configured input file is a BAM: each read is
/// projected onto reference coordinates and ingested through the same append +
/// finalize path. The table schema and reference are declared exactly as for
/// NDJSON preprocessing (database_config + reference_genome).
Database preprocessingBam(
   const config::PreprocessingConfig& preprocessing_config,
   append::bam::BamIngestOptions options = {}
);

}  // namespace rhydb::preprocessing
