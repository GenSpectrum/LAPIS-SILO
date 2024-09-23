#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>

#include "silo/config/util/abstract_config_source.h"

namespace silo::config {

const AbstractConfigSource::Option INPUT_DIRECTORY_OPTION{{"inputDirectory"}};
const AbstractConfigSource::Option OUTPUT_DIRECTORY_OPTION = {{"outputDirectory"}};
const AbstractConfigSource::Option INTERMEDIATE_RESULTS_DIRECTORY_OPTION = {
   {"intermediateResultsDirectory"}
};
const AbstractConfigSource::Option PREPROCESSING_DATABASE_LOCATION_OPTION = {
   {"preprocessingDatabaseLocation"}
};
const AbstractConfigSource::Option DUCKDB_MEMORY_LIMIT_OPTION = {{"duckdbMemoryLimitInG"}};
const AbstractConfigSource::Option LINEAGE_DEFINITIONS_FILENAME_OPTION = {
   {"lineageDefinitionsFilename"}
};
const AbstractConfigSource::Option NDJSON_INPUT_FILENAME_OPTION = {{"ndjsonInputFilename"}};
const AbstractConfigSource::Option REFERENCE_GENOME_FILENAME_OPTION = {{"referenceGenomeFilename"}};

const std::string DEFAULT_OUTPUT_DIRECTORY = "./output/";

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
   std::filesystem::path input_directory = "./";
   std::filesystem::path output_directory = DEFAULT_OUTPUT_DIRECTORY;
   std::filesystem::path intermediate_results_directory = "./temp/";
   std::optional<std::filesystem::path> preprocessing_database_location;
   std::optional<uint32_t> duckdb_memory_limit_in_g;
   std::optional<std::filesystem::path> lineage_definitions_file;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::filesystem::path reference_genome_file = "reference_genomes.json";

   void validate() const;

   [[nodiscard]] std::filesystem::path getOutputDirectory() const;

   [[nodiscard]] std::filesystem::path getIntermediateResultsDirectory() const;

   [[nodiscard]] std::optional<std::filesystem::path> getLineageDefinitionsFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPreprocessingDatabaseLocation() const;

   [[nodiscard]] std::optional<uint32_t> getDuckdbMemoryLimitInG() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   void overwrite(const silo::config::AbstractConfigSource& config_source);
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::PreprocessingConfig>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::PreprocessingConfig& preprocessing_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
