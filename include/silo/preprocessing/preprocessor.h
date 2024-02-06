#pragma once

#include "silo/config/database_config.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/storage/reference_genomes.h"

namespace silo {
class Database;
class PangoLineageAliasLookup;

namespace preprocessing {

class SequenceInfo;

class Preprocessor {
   PreprocessingConfig preprocessing_config;
   config::DatabaseConfig database_config;
   PreprocessingDatabase preprocessing_db;
   ReferenceGenomes reference_genomes_;

  public:
   Preprocessor(
      const preprocessing::PreprocessingConfig preprocessing_config,
      const config::DatabaseConfig database_config,
      const ReferenceGenomes& reference_genomes
   );

   Database preprocess();

  private:
   void buildTablesFromNdjsonInput(const std::filesystem::path& file_name);
   void buildMetadataTableFromFile(const std::filesystem::path& metadata_filename);

   void buildPartitioningTable();
   void buildPartitioningTableByColumn(const std::string& partition_by_field);
   void buildEmptyPartitioning();

   void createPartitionedSequenceTablesFromNdjson(const std::filesystem::path& file_name);

   void createAlignedPartitionedSequenceViews(
      const std::filesystem::path& file_name,
      const SequenceInfo& sequence_info,
      const std::string& partition_by_select,
      const std::string& partition_by_where
   );
   void createUnalignedPartitionedSequenceFiles(
      const std::filesystem::path& file_name,
      const std::string& partition_by_select,
      const std::string& partition_by_where
   );
   void createUnalignedPartitionedSequenceFile(
      const std::string& seq_name,
      const std::string& table_sql
   );

   void createPartitionedSequenceTablesFromSequenceFiles();
   void createPartitionedTableForSequence(
      const std::string& sequence_name,
      const std::string& reference_sequence,
      const std::filesystem::path& filename,
      const std::string& table_prefix
   );

   Database buildDatabase(
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause,
      const silo::PangoLineageAliasLookup& alias_key,
      const std::filesystem::path& intermediate_results_directory
   );

   void buildMetadataStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );
   void buildNucleotideSequenceStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );
   void buildAminoAcidSequenceStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );
};
}  // namespace preprocessing
}  // namespace silo
