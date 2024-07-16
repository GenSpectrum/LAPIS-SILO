#pragma once

#include <optional>

#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"

namespace silo {
class Database;

namespace preprocessing {

class SequenceInfo;
class ValidatedNdjsonFile;

class Preprocessor {
   config::PreprocessingConfig preprocessing_config;
   config::DatabaseConfig database_config;
   PreprocessingDatabase preprocessing_db;
   ReferenceGenomes reference_genomes_;
   PangoLineageAliasLookup alias_lookup_;

   std::vector<std::string> nuc_sequences;
   std::vector<std::string> aa_sequences;
   std::vector<std::string> order_by_fields;
   std::vector<std::string> prefixed_order_by_fields;
   std::vector<std::string> prefixed_nuc_sequences;
   std::vector<std::string> prefixed_aa_sequences;
   std::vector<std::string> prefixed_nuc_insertions_fields;
   std::vector<std::string> prefixed_aa_insertions_fields;

  public:
   Preprocessor(
      config::PreprocessingConfig preprocessing_config,
      config::DatabaseConfig database_config,
      const ReferenceGenomes& reference_genomes,
      PangoLineageAliasLookup alias_lookup
   );

   Database preprocess();

  private:
   void validateConfig();

   static std::string makeNonNullKey(const std::string& field);
   std::string getPartitionKeySelect() const;

   void buildTablesFromNdjsonInput(const ValidatedNdjsonFile& input_file);
   void buildMetadataTableFromFile(const std::filesystem::path& metadata_filename);

   void buildPartitioningTable();
   void buildPartitioningTableByColumn(const std::string& partition_by_field);
   void buildEmptyPartitioning();

   void createInsertionsTableFromFile(
      const std::vector<std::string>& expected_sequences,
      const std::filesystem::path& insertion_file,
      const std::string& table_name
   );

   void createPartitionedSequenceTablesFromNdjson(const ValidatedNdjsonFile& input_file);

   void createAlignedPartitionedSequenceViews(const ValidatedNdjsonFile& input_file);
   void createUnalignedPartitionedSequenceFiles(const ValidatedNdjsonFile& input_file);
   void createUnalignedPartitionedSequenceFile(
      const std::string& seq_name,
      const std::string& table_sql
   );

   void createPartitionedSequenceTablesFromSequenceFiles();

   template <typename SymbolType>
   void createPartitionedTableForSequence(
      const std::string& sequence_name,
      const std::string& reference_sequence,
      const std::filesystem::path& filename
   );

   Database buildDatabase(
      const preprocessing::Partitions& partition_descriptor,
      const std::filesystem::path& intermediate_results_directory
   );

   void buildMetadataStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );

   template <typename SymbolType>
   void buildSequenceStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );
};
}  // namespace preprocessing
}  // namespace silo
