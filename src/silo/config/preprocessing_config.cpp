#include "silo/config/preprocessing_config.h"

#include <filesystem>
#include <system_error>

#include <spdlog/spdlog.h>

#include "silo/common/fmt_formatters.h"
#include "silo/config/util/abstract_config_source.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo_api/command_line_arguments.h"

namespace silo::config {

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

void PreprocessingConfig::addOptions(Poco::Util::OptionSet& options) {
#define TUPLE(                                                    \
   TYPE,                                                          \
   FIELD_NAME,                                                    \
   DEFAULT_GENERATION,                                            \
   DEFAULT_VALUE,                                                 \
   OPTION_PATH,                                                   \
   PARSING_ACCESSOR_TYPE_NAME,                                    \
   HELP_TEXT,                                                     \
   ACCESSOR_GENERATION,                                           \
   ACCESSOR_NAME                                                  \
)                                                                 \
   {                                                              \
      const AbstractConfigSource::Option opt{OPTION_PATH};        \
      std::string option_string = toUnix(opt);                    \
      options.addOption(Poco::Util::Option()                      \
                           .fullName(option_string)               \
                           .description(HELP_TEXT)                \
                           .required(false)                       \
                           .repeatable(false)                     \
                           .argument(#PARSING_ACCESSOR_TYPE_NAME) \
                           .binding(option_string));              \
   }

   PREPROCESSING_CONFIG_DEFINITION;

#undef TUPLE
}

void PreprocessingConfig::overwrite(const silo::config::AbstractConfigSource& config_source) {
#define TUPLE(                                                                                  \
   TYPE,                                                                                        \
   FIELD_NAME,                                                                                  \
   DEFAULT_GENERATION,                                                                          \
   DEFAULT_VALUE,                                                                               \
   OPTION_PATH,                                                                                 \
   PARSING_ACCESSOR_TYPE_NAME,                                                                  \
   HELP_TEXT,                                                                                   \
   ACCESSOR_GENERATION,                                                                         \
   ACCESSOR_NAME                                                                                \
)                                                                                               \
   {                                                                                            \
      const AbstractConfigSource::Option opt{OPTION_PATH};                                      \
      if (auto value = config_source.get##PARSING_ACCESSOR_TYPE_NAME(opt)) {                    \
         SPDLOG_DEBUG(                                                                          \
            "Using {} as passed via {}: {}", opt.toString(), config_source.configType(), *value \
         );                                                                                     \
         (FIELD_NAME) = *value;                                                                 \
      }                                                                                         \
   }

   PREPROCESSING_CONFIG_DEFINITION;

#undef TUPLE
}

}  // namespace silo::config

[[maybe_unused]] auto fmt::formatter<silo::config::PreprocessingConfig>::format(
   const silo::config::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   fmt::format_to(ctx.out(), "{{\n");
   const char* perhaps_comma = " ";

#define TUPLE(                                                                              \
   TYPE,                                                                                    \
   FIELD_NAME,                                                                              \
   DEFAULT_GENERATION,                                                                      \
   DEFAULT_VALUE,                                                                           \
   OPTION_PATH,                                                                             \
   PARSING_ACCESSOR_TYPE_NAME,                                                              \
   HELP_TEXT,                                                                               \
   ACCESSOR_GENERATION,                                                                     \
   ACCESSOR_NAME                                                                            \
)                                                                                           \
   fmt::format_to(                                                                          \
      ctx.out(), "{} {}: ''", perhaps_comma, "#FIELD_NAME", preprocessing_config.FIELD_NAME \
   );                                                                                       \
   perhaps_comma = ",";

   PREPROCESSING_CONFIG_DEFINITION;

#undef TUPLE

   return fmt::format_to(ctx.out(), "}}\n");
}
