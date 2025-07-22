#include "silo/initialize/initializer.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_invoke.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/block_timer.h"
#include "silo/common/fmt_formatters.h"
#include "silo/common/panic.h"
#include "silo/common/phylo_tree.h"
#include "silo/common/string_utils.h"
#include "silo/common/table_reader.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/initialize/initialize_exception.h"
#include "silo/storage/column/column_type_visitor.h"
#include "silo/storage/column/zstd_compressed_string_column.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::initialize {

Database Initializer::initializeDatabase(const config::InitializationFiles& initialization_files) {
   EVOBENCH_SCOPE("Initializer", "initializeDatabase");
   common::LineageTreeAndIdMap lineage_tree;
   if (initialization_files.getLineageDefinitionsFilename().has_value()) {
      lineage_tree = common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
         initialization_files.getLineageDefinitionsFilename().value()
      );
   }

   common::PhyloTree phylo_tree_file;
   auto opt_path = initialization_files.getPhyloTreeFilename();
   if (opt_path.has_value()) {
      phylo_tree_file = common::PhyloTree::fromFile(opt_path.value());
   }
   silo::schema::DatabaseSchema schema = createSchemaFromConfigFiles(
      config::DatabaseConfig::getValidatedConfigFromFile(
         initialization_files.getDatabaseConfigFilename()
      ),
      ReferenceGenomes::readFromFile(initialization_files.getReferenceGenomeFilename()),
      std::move(lineage_tree),
      std::move(phylo_tree_file)
   );
   return Database{schema};
}

struct ColumnMetadataInitializer {
   template <storage::column::Column ColumnType>
   void operator()(
      std::shared_ptr<storage::column::ColumnMetadata>& metadata,
      const config::DatabaseMetadata& config_metadata,
      const ReferenceGenomes& reference_genomes,
      const common::LineageTreeAndIdMap& lineage_tree,
      const common::PhyloTree& phylo_tree_file
   );
};

template <>
void ColumnMetadataInitializer::operator()<storage::column::IndexedStringColumnPartition>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& reference_genomes,
   const common::LineageTreeAndIdMap& lineage_tree,
   const common::PhyloTree& phylo_tree_file
) {
   if (config_metadata.generate_lineage_index) {
      metadata = std::make_shared<storage::column::IndexedStringColumnPartition::Metadata>(
         config_metadata.name, lineage_tree
      );
   } else {
      metadata = std::make_shared<storage::column::IndexedStringColumnPartition::Metadata>(
         config_metadata.name
      );
   }
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::StringColumnPartition>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& reference_genomes,
   const common::LineageTreeAndIdMap& lineage_tree,
   const common::PhyloTree& phylo_tree_file
) {
   if (config_metadata.phylo_tree_node_identifier) {
      metadata = std::make_shared<storage::column::StringColumnPartition::Metadata>(
         config_metadata.name, phylo_tree_file
      );
   } else {
      metadata =
         std::make_shared<storage::column::StringColumnPartition::Metadata>(config_metadata.name);
   }
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::ZstdCompressedStringColumnPartition>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& reference_genomes,
   const common::LineageTreeAndIdMap& lineage_tree,
   const common::PhyloTree& phylo_tree_file
) {
   SILO_PANIC("unaligned nucleotide sequences cannot be in config::DatabaseMetadata");
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::SequenceColumnPartition<Nucleotide>>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& reference_genomes,
   const common::LineageTreeAndIdMap& lineage_tree,
   const common::PhyloTree& phylo_tree_file
) {
   SILO_PANIC("nucleotides cannot be in config::DatabaseMetadata");
}

template <>
void ColumnMetadataInitializer::operator()<storage::column::SequenceColumnPartition<AminoAcid>>(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& reference_genomes,
   const common::LineageTreeAndIdMap& lineage_tree,
   const common::PhyloTree& phylo_tree_file
) {
   SILO_PANIC("amino acid cannot be in config::DatabaseMetadata");
}

template <storage::column::Column ColumnType>
void ColumnMetadataInitializer::operator()(
   std::shared_ptr<storage::column::ColumnMetadata>& metadata,
   const config::DatabaseMetadata& config_metadata,
   const ReferenceGenomes& /*reference_genomes*/,
   const common::LineageTreeAndIdMap& /*lineage_tree*/,
   const common::PhyloTree& /*phylo_tree_file*/
) {
   metadata = std::make_shared<typename ColumnType::Metadata>(config_metadata.name);
}

namespace {

void setDefaultSequencesIfUnsetAndThereIsOnlyOne(
   silo::config::DatabaseConfig& database_config,
   const ReferenceGenomes& reference_genomes
) {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   if (nuc_sequence_names.size() == 1 && !database_config.default_nucleotide_sequence.has_value()) {
      database_config.default_nucleotide_sequence = nuc_sequence_names.at(0);
   }
   if (aa_sequence_names.size() == 1 && !database_config.default_amino_acid_sequence.has_value()) {
      database_config.default_amino_acid_sequence = aa_sequence_names.at(0);
   }
}

void assertDefaultSequencesAreInReference(
   const silo::config::DatabaseConfig& database_config,
   const ReferenceGenomes& reference_genomes
) {
   const auto& nuc_sequence_names = reference_genomes.getSequenceNames<Nucleotide>();
   const auto& aa_sequence_names = reference_genomes.getSequenceNames<AminoAcid>();
   const bool default_nucleotide_sequence_is_not_in_reference =
      database_config.default_nucleotide_sequence.has_value() &&
      std::ranges::find(nuc_sequence_names, *database_config.default_nucleotide_sequence) ==
         nuc_sequence_names.end();
   if (default_nucleotide_sequence_is_not_in_reference) {
      throw silo::initialize::InitializeException(
         "The default nucleotide sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
   const bool default_amino_acid_sequence_is_not_in_reference =
      database_config.default_amino_acid_sequence.has_value() &&
      std::ranges::find(aa_sequence_names, *database_config.default_amino_acid_sequence) ==
         aa_sequence_names.end();
   if (default_amino_acid_sequence_is_not_in_reference) {
      throw silo::initialize::InitializeException(
         "The default amino acid sequence that is set in the database config is not contained in "
         "the reference genomes."
      );
   }
}

void assertPrimaryKeyInMetadata(const silo::config::DatabaseConfig& database_config) {
   auto primary_key_metadata = std::ranges::find_if(
      database_config.schema.metadata,
      [&database_config](const auto& metadata) {
         return database_config.schema.primary_key == metadata.name;
      }
   );
   if (primary_key_metadata == database_config.schema.metadata.end()) {
      throw silo::initialize::InitializeException(
         "The primary key is not contained in the metadata."
      );
   }
}

void assertPrimaryKeyOfTypeString(const silo::config::DatabaseConfig& database_config) {
   auto primary_key_metadata = std::ranges::find_if(
      database_config.schema.metadata,
      [&database_config](const auto& metadata) {
         return database_config.schema.primary_key == metadata.name;
      }
   );
   auto primary_key_type = primary_key_metadata->getColumnType();
   if (primary_key_type != schema::ColumnType::STRING) {
      throw silo::initialize::InitializeException("The primary key must be of type STRING.");
   }
}

}  // namespace

silo::schema::DatabaseSchema Initializer::createSchemaFromConfigFiles(
   config::DatabaseConfig database_config,
   ReferenceGenomes reference_genomes,
   common::LineageTreeAndIdMap lineage_tree,
   common::PhyloTree phylo_tree_file
) {
   setDefaultSequencesIfUnsetAndThereIsOnlyOne(database_config, reference_genomes);
   assertDefaultSequencesAreInReference(database_config, reference_genomes);
   assertPrimaryKeyInMetadata(database_config);
   assertPrimaryKeyOfTypeString(database_config);

   schema::ColumnIdentifier primary_key{
      database_config.schema.primary_key, schema::ColumnType::STRING
   };

   std::map<schema::ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>>
      column_metadata;
   for (const auto& config_metadata : database_config.schema.metadata) {
      schema::ColumnIdentifier column_identifier{
         .name = config_metadata.name, .type = config_metadata.getColumnType()
      };
      std::shared_ptr<storage::column::ColumnMetadata> metadata;
      storage::column::visit(
         column_identifier.type,
         ColumnMetadataInitializer{},
         metadata,
         config_metadata,
         reference_genomes,
         lineage_tree,
         phylo_tree_file
      );
      column_metadata.emplace(column_identifier, metadata);
   }

   for (size_t sequence_idx = 0; sequence_idx < reference_genomes.nucleotide_sequence_names.size();
        ++sequence_idx) {
      const auto& sequence_name = reference_genomes.nucleotide_sequence_names.at(sequence_idx);
      const auto& reference_sequence = reference_genomes.raw_nucleotide_sequences.at(sequence_idx);
      schema::ColumnIdentifier column_identifier{
         sequence_name, schema::ColumnType::NUCLEOTIDE_SEQUENCE
      };
      auto metadata = std::make_shared<storage::column::SequenceColumnMetadata<Nucleotide>>(
         sequence_name, ReferenceGenomes::stringToVector<Nucleotide>(reference_sequence)
      );
      column_metadata.emplace(column_identifier, std::move(metadata));
      schema::ColumnIdentifier column_identifier_unaligned{
         silo::storage::UNALIGNED_NUCLEOTIDE_SEQUENCE_PREFIX + sequence_name,
         schema::ColumnType::ZSTD_COMPRESSED_STRING
      };
      auto metadata_unaligned =
         std::make_shared<storage::column::ZstdCompressedStringColumnMetadata>(
            sequence_name, reference_sequence
         );
      column_metadata.emplace(column_identifier_unaligned, std::move(metadata_unaligned));
   }

   for (size_t sequence_idx = 0; sequence_idx < reference_genomes.aa_sequence_names.size();
        ++sequence_idx) {
      const auto& sequence_name = reference_genomes.aa_sequence_names.at(sequence_idx);
      const auto& reference_sequence = reference_genomes.raw_aa_sequences.at(sequence_idx);
      schema::ColumnIdentifier column_identifier{
         sequence_name, schema::ColumnType::AMINO_ACID_SEQUENCE
      };
      auto metadata = std::make_shared<storage::column::SequenceColumnMetadata<AminoAcid>>(
         sequence_name, ReferenceGenomes::stringToVector<AminoAcid>(reference_sequence)
      );
      column_metadata.emplace(column_identifier, std::move(metadata));
   }

   silo::schema::TableSchema table_schema{column_metadata, primary_key};
   if (database_config.default_nucleotide_sequence.has_value()) {
      table_schema.default_nucleotide_sequence = schema::ColumnIdentifier{
         .name = database_config.default_nucleotide_sequence.value(),
         .type = schema::ColumnType::NUCLEOTIDE_SEQUENCE
      };
   }
   if (database_config.default_amino_acid_sequence.has_value()) {
      table_schema.default_aa_sequence = schema::ColumnIdentifier{
         .name = database_config.default_amino_acid_sequence.value(),
         .type = schema::ColumnType::AMINO_ACID_SEQUENCE
      };
   }
   return silo::schema::DatabaseSchema{.tables = {{schema::TableName{"default"}, table_schema}}};
}

}  // namespace silo::initialize
