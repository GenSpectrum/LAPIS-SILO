#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace silo::preprocessing {
struct PreprocessingConfig;

struct OptionalPreprocessingConfig {
   /**
    * The directory where the input files are located.
    */
   std::optional<std::filesystem::path> input_directory;
   /**
    * The directory where the output files will be located.
    */
   std::optional<std::filesystem::path> output_directory;
   /**
    * The directory where the intermediate results will be stored
    * that are not relevant for an end user.
    */
   std::optional<std::filesystem::path> intermediate_results_directory;
   /**
    * The filename of the ndjson input file, relative to the inputDirectory
    * Must not be specified together with metadata input file
    */
   std::optional<std::filesystem::path> ndjson_input_filename;
   /**
    * The filename of the metadata file, relative to the inputDirectory
    */
   std::optional<std::filesystem::path> metadata_file;
   /**
    * The filename of the pango lineage definition file, relative to the inputDirectory
    */
   std::optional<std::filesystem::path> pango_lineage_definition_file;
   /**
    * Folder for intermediate partition files, relative to the intermediateResultsDirectory
    */
   std::optional<std::filesystem::path> partition_folder;
   /**
    * Folder for intermediate sorted partition files, relative to the intermediateResultsDirectory
    */
   std::optional<std::filesystem::path> sorted_partition_folder;
   /**
    * The filename of the reference genome file, relative to the inputDirectory
    */
   std::optional<std::filesystem::path> reference_genome_file;
   /**
    * Prefix that SILO expects for nucleotide sequence files
    */
   std::optional<std::string> nucleotide_sequence_prefix;
   /**
    * Prefix that SILO expects for gene sequence files
    */
   std::optional<std::string> gene_prefix;

   PreprocessingConfig mergeValuesFromOrDefault(const OptionalPreprocessingConfig& other) const;
};

class PreprocessingConfigReader {
  public:
   virtual OptionalPreprocessingConfig readConfig(const std::filesystem::path& config_path) const;
};
}  // namespace silo::preprocessing
