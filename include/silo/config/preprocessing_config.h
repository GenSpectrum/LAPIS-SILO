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
const AbstractConfigSource::Option PANGO_LINEAGE_DEFINITION_FILENAME_OPTION = {
   {"pangoLineageDefinitionFilename"}
};
const AbstractConfigSource::Option NDJSON_INPUT_FILENAME_OPTION = {{"ndjsonInputFilename"}};
const AbstractConfigSource::Option METADATA_FILENAME_OPTION = {{"metadataFilename"}};
const AbstractConfigSource::Option REFERENCE_GENOME_FILENAME_OPTION = {{"referenceGenomeFilename"}};
const AbstractConfigSource::Option NUCLEOTIDE_SEQUENCE_PREFIX_OPTION = {{"nucleotideSequencePrefix"}
};
const AbstractConfigSource::Option UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX_OPTION = {
   {"unalignedNucleotideSequencePrefix"}
};
const AbstractConfigSource::Option GENE_PREFIX_OPTION = {{"genePrefix"}};
const AbstractConfigSource::Option NUCLEOTIDE_INSERTIONS_OPTION = {{"nucleotideInsertionsFilename"}
};
const AbstractConfigSource::Option AMINO_ACID_INSERTIONS_OPTION = {{"aminoAcidInsertionsFilename"}};

static const std::string DEFAULT_OUTPUT_DIRECTORY = "./output/";

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
   std::filesystem::path input_directory = "./";
   std::filesystem::path output_directory = DEFAULT_OUTPUT_DIRECTORY;
   std::filesystem::path intermediate_results_directory = "./temp/";
   std::optional<std::filesystem::path> preprocessing_database_location;
   std::optional<std::filesystem::path> pango_lineage_definition_file;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::optional<std::filesystem::path> metadata_file;
   std::filesystem::path reference_genome_file = "reference_genomes.json";
   std::string nucleotide_sequence_prefix = "nuc_";
   std::string unaligned_nucleotide_sequence_prefix = "unaligned_";
   std::string gene_prefix = "gene_";
   std::string nuc_insertions_filename = "nuc_insertions.tsv";
   std::string aa_insertions_filename = "aa_insertions.tsv";

   void validate() const;

   [[nodiscard]] std::filesystem::path getOutputDirectory() const;

   [[nodiscard]] std::filesystem::path getIntermediateResultsDirectory() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPangoLineageDefinitionFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPreprocessingDatabaseLocation() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getMetadataInputFilename() const;

   [[nodiscard]] std::filesystem::path getNucFilenameNoExtension(std::string_view nuc_name) const;

   [[nodiscard]] std::filesystem::path getUnalignedNucFilenameNoExtension(std::string_view nuc_name
   ) const;

   [[nodiscard]] std::filesystem::path getGeneFilenameNoExtension(std::string_view gene_name) const;

   [[nodiscard]] std::filesystem::path getNucleotideInsertionsFilename() const;

   [[nodiscard]] std::filesystem::path getAminoAcidInsertionsFilename() const;

   void overwrite(const silo::config::AbstractConfigSource& config_reader);
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
