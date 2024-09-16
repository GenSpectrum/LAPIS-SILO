#pragma once

#include <optional>

#include "silo/common/table_reader.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/preprocessing/identifiers.h"
#include "silo/preprocessing/preprocessing_database.h"
#include "silo/preprocessing/validated_ndjson_file.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::preprocessing {

class Preprocessor {
   config::PreprocessingConfig preprocessing_config;
   config::DatabaseConfig database_config;
   ReferenceGenomes reference_genomes;
   PangoLineageAliasLookup alias_lookup;

   PreprocessingDatabase preprocessing_db;

   Identifiers nuc_sequence_identifiers_without_prefix;
   Identifiers aa_sequence_identifiers_without_prefix;
   Identifiers nuc_sequence_identifiers;
   Identifiers aa_sequence_identifiers;
   Identifiers unaligned_nuc_sequences;
   Identifiers order_by_fields_without_prefix;
   Identifiers order_by_fields;
   Identifiers nuc_insertions_fields;
   Identifiers aa_insertions_fields;

  public:
   Preprocessor(
      config::PreprocessingConfig preprocessing_config,
      config::DatabaseConfig database_config,
      ReferenceGenomes reference_genomes,
      PangoLineageAliasLookup alias_lookup
   );

   Database preprocess();

  private:
   template <typename SymbolType>
   Identifiers getSequenceIdentifiers();

   void finalizeConfig();
   void validateConfig();

   static std::string makeNonNullKey(const std::string& field);
   std::string getPartitionKeySelect() const;

   void buildTablesFromNdjsonInput(const ValidatedNdjsonFile& input_file);
   void buildMetadataTableFromFile(const std::filesystem::path& metadata_filename);

   void buildPartitioningTable();
   void buildPartitioningTableByColumn(const std::string& partition_by_field);
   void buildEmptyPartitioning();

   template <typename SymbolType>
   Identifiers getInsertionsFields();

   template <typename SymbolType>
   void createInsertionsTableFromFile(const std::filesystem::path& insertion_file);

   void createPartitionedSequenceTablesFromNdjson(const ValidatedNdjsonFile& input_file);

   void createAlignedPartitionedSequenceViews(const ValidatedNdjsonFile& input_file);
   void createUnalignedPartitionedSequenceFiles(const ValidatedNdjsonFile& input_file);
   void createUnalignedPartitionedSequenceFile(size_t sequence_idx, const std::string& table_sql);

   void createPartitionedSequenceTablesFromSequenceFiles();

   template <typename SymbolType>
   void createPartitionedTableForSequence(
      size_t sequence_idx,
      const Identifier& prefixed_sequence_identifier,
      const std::string& compression_dictionary,
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
   ColumnFunction createRawReadLambda(
      ZstdDecompressor& decompressor,
      silo::SequenceStorePartition<SymbolType>& sequence_store
   );

   template <typename SymbolType>
   ColumnFunction createInsertionLambda(
      const std::string& sequence_name,
      silo::SequenceStorePartition<SymbolType>& sequence_store
   );

   template <typename SymbolType>
   void buildSequenceStore(
      Database& database,
      const preprocessing::Partitions& partition_descriptor,
      const std::string& order_by_clause
   );
};
}  // namespace silo::preprocessing
