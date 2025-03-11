#include "silo/initialize/initializer.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_invoke.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/fmt_formatters.h"
#include "silo/common/panic.h"
#include "silo/common/string_utils.h"
#include "silo/common/table_reader.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/validated_ndjson_file.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/unaligned_sequence_store.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::initialize {

Initializer::Initializer(
   config::InitializeConfig initialize_config_,
   config::DatabaseConfig database_config_,
   ReferenceGenomes reference_genomes_,
   common::LineageTreeAndIdMap lineage_tree_
)
    : initialize_config(std::move(initialize_config_)),
      database_config(std::move(database_config_)),
      reference_genomes(std::move(reference_genomes_)),
      lineage_tree(std::move(lineage_tree_)) {}

Database Initializer::initialize() {
   finalizeConfig();
   Database database{
      silo::config::DatabaseConfig{database_config},
      std::move(lineage_tree),
      reference_genomes.getSequenceNames<Nucleotide>(),
      reference_genomes.getReferenceSequences<Nucleotide>(),
      reference_genomes.getSequenceNames<AminoAcid>(),
      reference_genomes.getReferenceSequences<AminoAcid>()
   };
   const DataVersion& data_version = DataVersion::mineDataVersion();
   SPDLOG_INFO("preprocessing - mining data data_version: {}", data_version.toString());
   database.setDataVersion(data_version);
   return database;
}

void Initializer::finalizeConfig() {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   if (nuc_sequence_names.size() == 1 && !database_config.default_nucleotide_sequence.has_value()) {
      database_config.default_nucleotide_sequence = nuc_sequence_names.at(0);
   }
   if (aa_sequence_names.size() == 1 && !database_config.default_amino_acid_sequence.has_value()) {
      database_config.default_amino_acid_sequence = aa_sequence_names.at(0);
   }

   validateConfig();
}

void Initializer::validateConfig() {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   const bool default_nucleotide_sequence_is_not_in_reference =
      database_config.default_nucleotide_sequence.has_value() &&
      std::ranges::find(nuc_sequence_names, *database_config.default_nucleotide_sequence) ==
         nuc_sequence_names.end();
   if (default_nucleotide_sequence_is_not_in_reference) {
      throw silo::preprocessing::PreprocessingException(
         "The default nucleotide sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
   const bool default_amino_acid_sequence_is_not_in_reference =
      database_config.default_amino_acid_sequence.has_value() &&
      std::ranges::find(aa_sequence_names, *database_config.default_amino_acid_sequence) ==
         aa_sequence_names.end();
   if (default_amino_acid_sequence_is_not_in_reference) {
      throw silo::preprocessing::PreprocessingException(
         "The default amino acid sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
}

}  // namespace silo::initialize
