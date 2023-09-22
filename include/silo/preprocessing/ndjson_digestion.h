#pragma once

#include <string_view>

namespace duckdb {
struct Connection;
}

namespace silo {
struct ReferenceGenomes;
namespace preprocessing {
struct PreprocessingConfig;
}

void executeDuckDBRoutineForNdjsonDigestion(
   duckdb::Connection& connection,
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const silo::ReferenceGenomes& reference_genomes,
   std::string_view file_name,
   std::string_view primary_key_metadata_column
);

}  // namespace silo