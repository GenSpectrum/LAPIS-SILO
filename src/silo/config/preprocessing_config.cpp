#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/config/util/abstract_config_source.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo_api/command_line_arguments.h"

namespace silo::config {

const ConfigStruct PREPROCESSING_CONFIG_METADATA{
   "PreprocessingOptions",
   {ConfigStructField{"help", ConfigValue{
                         type_name : "bool",
                         default_value : {},
                         help_text : "Show help text.",
                      }},
    ConfigStructField{"runtimeConfig", ConfigValue{
                         type_name : "ignored",
                         default_value : {},
                         help_text :
                            "Ignored so that defaults can be provided via env vars for both \n"
                            "execution modes of the multi-call binary.",
                      }},
    ConfigStructField{"preprocessingConfig", ConfigValue{
                         type_name : "path",
                         default_value : {},
                         help_text : "Path to config file in YAML format.",
                      }},

    ConfigStructField{
       .field_name_camel = "inputDirectory",
       .value =
          ConfigValue{
             .type_name = "path",
             .default_value = {"./"},
             .help_text = "the path to the directory with the input files",
          }
    },
    ConfigStructField{
       .field_name_camel = "outputDirectory",
       .value =
          ConfigValue{
             .type_name = "path",
             .default_value = {DEFAULT_OUTPUT_DIRECTORY},
             .help_text = "the path to the directory to hold the output files",
          }
    },
    ConfigStructField{
       .field_name_camel = "intermediateResultsDirectory",
       .value =
          ConfigValue{
             .type_name = "path",
             .default_value = {"./temp/"},
             .help_text = "the path to the directory to hold temporary files",
          }
    },
    ConfigStructField{
       .field_name_camel = "preprocessingDatabaseLocation",
       .value =
          ConfigValue{
             .type_name = "Option<path>",
             .default_value = std::nullopt,
             .help_text = "XXX?",
          }
    },
    ConfigStructField{
       .field_name_camel = "duckdbMemoryLimitInG",
       .value =
          ConfigValue{
             .type_name = "Option<u32>",
             .default_value = std::nullopt,
             .help_text = "DuckDB memory limit in GB",
          }
    },
    ConfigStructField{
       .field_name_camel = "pangoLineageDefinitionFilename",
       .value =
          ConfigValue{
             .type_name = "Option<path>",
             .default_value = std::nullopt,
             .help_text = "file name of the file holding the pango lineage definitions",
          }
    },
    ConfigStructField{
       .field_name_camel = "ndjsonInputFilename",
       .value =
          ConfigValue{
             .type_name = "Option<path>",
             .default_value = std::nullopt,
             .help_text = "file name of the file holding NDJSON input",
          }
    },
    ConfigStructField{
       .field_name_camel = "metadataFilename",
       .value =
          ConfigValue{
             .type_name = "Option<path>",
             .default_value = std::nullopt,
             .help_text = "file name of the file holding metadata",
          }
    },
    ConfigStructField{
       .field_name_camel = "databaseConfigFile",
       .value =
          ConfigValue{
             .type_name = "path",
             .default_value = {"database_config.yaml"},
             .help_text = "file name of the file holding the database table configuration",
          }
    },
    ConfigStructField{
       .field_name_camel = "referenceGenomeFilename",
       .value =
          ConfigValue{
             .type_name = "path",
             .default_value = {"reference_genomes.json"},
             .help_text = "file name of the file holding the reference genome",
          }
    },
    ConfigStructField{
       .field_name_camel = "nucleotideSequencePrefix",
       .value =
          ConfigValue{
             .type_name = "string",
             .default_value = {"nuc_"},
             .help_text = "the prefix for nucleotide sequences",
          }
    },
    ConfigStructField{
       .field_name_camel = "unalignedNucleotideSequencePrefix",
       .value =
          ConfigValue{
             .type_name = "string",
             .default_value = {"unaligned_"},
             .help_text = "the prefix for unaligned nucleotide sequences",
          }
    },
    ConfigStructField{
       .field_name_camel = "genePrefix",
       .value =
          ConfigValue{
             .type_name = "string",
             .default_value = {"gene_"},
             .help_text = "the prefix for genes XX?",
          }
    },
    ConfigStructField{
       .field_name_camel = "nucleotideInsertionsFilename",
       .value =
          ConfigValue{
             .type_name = "string",
             .default_value = {"nuc_insertions.tsv"},
             .help_text = "the file name of the file holding nucleotide insertions",
          }
    },
    ConfigStructField{
       .field_name_camel = "aminoAcidInsertionsFilename",
       .value =
          ConfigValue{
             .type_name = "string",
             .default_value = {"aa_insertions.tsv"},
             .help_text = "the file name of the file hodling amino acid insertions",
          }
    }}
};

void PreprocessingConfig::validate() const {
   if (!std::filesystem::exists(input_directory)) {
      throw preprocessing::PreprocessingException(input_directory.string() + " does not exist");
   }
   if (ndjson_input_filename.has_value() && metadata_file.has_value()) {
      throw preprocessing::PreprocessingException(fmt::format(
         "Cannot specify both a ndjsonInputFilename ('{}') and metadataFilename('{}').",
         ndjson_input_filename.value().string(),
         metadata_file.value().string()
      ));
   }
   if (!ndjson_input_filename.has_value() && !metadata_file.has_value()) {
      throw preprocessing::PreprocessingException(
         fmt::format("Neither a ndjsonInputFilename nor a metadataFilename was specified as "
                     "preprocessing option.")
      );
   }
}

std::filesystem::path PreprocessingConfig::getDatabaseConfigFilename() const {
   return input_directory / database_config_file;
}

std::optional<std::filesystem::path> PreprocessingConfig::getPangoLineageDefinitionFilename(
) const {
   return pango_lineage_definition_file.has_value()
             ? std::optional(input_directory / *pango_lineage_definition_file)
             : std::nullopt;
}

std::filesystem::path PreprocessingConfig::getReferenceGenomeFilename() const {
   return input_directory / reference_genome_file;
}

std::optional<std::filesystem::path> PreprocessingConfig::getMetadataInputFilename() const {
   return metadata_file.has_value() ? std::optional(input_directory / *metadata_file)
                                    : std::nullopt;
}

std::optional<std::filesystem::path> PreprocessingConfig::getNdjsonInputFilename() const {
   return ndjson_input_filename.has_value()
             ? std::optional(input_directory / *ndjson_input_filename)
             : std::nullopt;
}

std::filesystem::path PreprocessingConfig::getNucFilenameNoExtension(size_t sequence_idx) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", nucleotide_sequence_prefix, sequence_idx);
   return filename;
}

std::filesystem::path PreprocessingConfig::getUnalignedNucFilenameNoExtension(size_t sequence_idx
) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", unaligned_nucleotide_sequence_prefix, sequence_idx);
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneFilenameNoExtension(size_t sequence_idx) const {
   std::filesystem::path filename = input_directory;
   filename /= fmt::format("{}{}", gene_prefix, sequence_idx);
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucleotideInsertionsFilename() const {
   return input_directory / nuc_insertions_filename;
}

std::filesystem::path PreprocessingConfig::getAminoAcidInsertionsFilename() const {
   return input_directory / aa_insertions_filename;
}

namespace {
std::string toUnix(const AbstractConfigSource::Option& option) {
   return silo_api::CommandLineArguments::asUnixOptionString(option);
}
}  // namespace

void PreprocessingConfig::overwriteFromParents(
   const ConsList<std::string>& parents,
   const VerifiedConfigSource& config_source
) {
   using ::config::config_source_interface::get;
   using ::config::config_source_interface::set;

   set<bool, bool>(help, config_source, parents, "help");
   set<Ignored, decltype(runtime_config)>(runtime_config, config_source, parents, "runtimeConfig");
   set<std::filesystem::path, decltype(preprocessing_config)>(
      preprocessing_config, config_source, parents, "preprocessingConfig"
   );

   set<std::filesystem::path, decltype(input_directory)>(
      input_directory, config_source, parents, "inputDirectory"
   );
   set<std::filesystem::path, decltype(output_directory)>(
      output_directory, config_source, parents, "outputDirectory"
   );
   set<std::filesystem::path, decltype(intermediate_results_directory)>(
      intermediate_results_directory, config_source, parents, "intermediateResultsDirectory"
   );
   set<std::filesystem::path, decltype(preprocessing_database_location)>(
      preprocessing_database_location, config_source, parents, "preprocessingDatabaseLocation"
   );
   set<uint32_t, decltype(duckdb_memory_limit_in_g)>(
      duckdb_memory_limit_in_g, config_source, parents, "duckdbMemoryLimitInG"
   );
   set<std::filesystem::path, decltype(pango_lineage_definition_file)>(
      pango_lineage_definition_file, config_source, parents, "pangoLineageDefinitionFilename"
   );
   set<std::filesystem::path, decltype(ndjson_input_filename)>(
      ndjson_input_filename, config_source, parents, "ndjsonInputFilename"
   );
   set<std::filesystem::path, decltype(metadata_file)>(
      metadata_file, config_source, parents, "metadataFilename"
   );
   set<std::filesystem::path, decltype(database_config_file)>(
      database_config_file, config_source, parents, "databaseConfigFile"
   );
   set<std::filesystem::path, decltype(reference_genome_file)>(
      reference_genome_file, config_source, parents, "referenceGenomeFilename"
   );
   set<std::string, decltype(nucleotide_sequence_prefix)>(
      nucleotide_sequence_prefix, config_source, parents, "nucleotideSequencePrefix"
   );
   set<std::string, decltype(unaligned_nucleotide_sequence_prefix)>(
      unaligned_nucleotide_sequence_prefix,
      config_source,
      parents,
      "unalignedNucleotideSequencePrefix"
   );
   set<std::string, decltype(gene_prefix)>(gene_prefix, config_source, parents, "genePrefix");
   set<std::string, decltype(nuc_insertions_filename)>(
      nuc_insertions_filename, config_source, parents, "nucleotideInsertionsFilename"
   );
   set<std::string, decltype(aa_insertions_filename)>(
      aa_insertions_filename, config_source, parents, "aminoAcidInsertionsFilename"
   );
}

bool PreprocessingConfig::asksForHelp() const {
   return help;
}
std::optional<std::filesystem::path> PreprocessingConfig::configPath() const {
   return preprocessing_config;
}

}  // namespace silo::config

// [[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
//    const silo::config::PreprocessingConfig& preprocessing_config,
//    fmt::format_context& ctx
// ) -> decltype(ctx.out()) {
//    fmt::format_to(ctx.out(), "{{\n");
//    const char* perhaps_comma = " ";

// #define TUPLE(                                                                              \
//    TYPE,                                                                                    \
//    FIELD_NAME,                                                                              \
//    DEFAULT_GENERATION,                                                                      \
//    DEFAULT_VALUE,                                                                           \
//    OPTION_PATH,                                                                             \
//    PARSING_ACCESSOR_TYPE_NAME,                                                              \
//    HELP_TEXT,                                                                               \
//    ACCESSOR_GENERATION,                                                                     \
//    ACCESSOR_NAME                                                                            \
// )                                                                                           \
//    fmt::format_to(                                                                          \
//       ctx.out(), "{} {}: ''", perhaps_comma, "#FIELD_NAME", preprocessing_config.FIELD_NAME \
//    );                                                                                       \
//    perhaps_comma = ",";

//    PREPROCESSING_CONFIG_DEFINITION;

// #undef TUPLE

//    return fmt::format_to(ctx.out(), "}}\n");
// }
