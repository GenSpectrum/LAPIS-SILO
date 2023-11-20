#include "silo/preprocessing/ndjson_digestion.h"

#include <fmt/format.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstdfasta/zstd_compressor.h"
#include "silo/zstdfasta/zstdfasta_writer.h"

void silo::executeDuckDBRoutineForNdjsonDigestion(
   duckdb::Connection& connection,
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name,
   std::string_view primary_key_metadata_column
) {
   // TODO delete
}
