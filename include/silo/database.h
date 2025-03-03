#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/data_version.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/silo_directory.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/storage/unaligned_sequence_store.h"

namespace silo {

class Database {
  public:
   silo::config::DatabaseConfig database_config;
   common::LineageTreeAndIdMap lineage_tree;
   std::vector<std::shared_ptr<DatabasePartition>> partitions;
   std::filesystem::path unaligned_sequences_directory;

   silo::storage::ColumnGroup columns;

   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

   std::map<std::string, SequenceStore<Nucleotide>> nuc_sequences;
   std::map<std::string, SequenceStore<AminoAcid>> aa_sequences;
   std::map<std::string, UnalignedSequenceStore> unaligned_nuc_sequences;

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database(
      silo::config::DatabaseConfig&& database_config,
      silo::common::LineageTreeAndIdMap&& lineage_tree,
      std::vector<std::string>&& nuc_sequence_names,
      std::vector<std::vector<Nucleotide::Symbol>>&& nuc_reference_sequences,
      std::vector<std::string>&& aa_sequence_names,
      std::vector<std::vector<AminoAcid::Symbol>>&& aa_reference_sequences
   );

   Database() = delete;

   Database(const Database&) = default;
   Database(Database&&) = default;
   Database& operator=(const Database&) = default;
   Database& operator=(Database&&) = default;

   virtual ~Database() = default;

   void validate() const;

   std::shared_ptr<DatabasePartition> addPartition();

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DetailedDatabaseInfo detailedDatabaseInfo() const;

   void setDataVersion(const DataVersion& data_version);
   virtual DataVersion::Timestamp getDataVersionTimestamp() const;

   template <typename SymbolType>
   std::optional<std::string> getDefaultSequenceName() const;

   template <typename SymbolType>
   std::vector<std::string> getSequenceNames() const;

   template <typename SymbolType>
   const std::map<std::string, SequenceStore<SymbolType>>& getSequenceStores() const;

   virtual query_engine::QueryResult executeQuery(const std::string& query) const;

  private:
   std::vector<std::vector<Nucleotide::Symbol>> getNucSequences() const;

   std::vector<std::vector<AminoAcid::Symbol>> getAASequences() const;

   void initializeColumns();
   void initializeColumn(const config::DatabaseMetadata& metadata);
   void addColumnToPartition(
      const config::DatabaseMetadata& metadata,
      std::shared_ptr<DatabasePartition>& partition
   );

   void initializeNucSequences(
      const std::vector<std::string>& sequence_names,
      std::vector<std::vector<Nucleotide::Symbol>>&& reference_sequences
   );
   void initializeAASequences(
      const std::vector<std::string>& sequence_names,
      std::vector<std::vector<AminoAcid::Symbol>>&& reference_sequences
   );

   template <typename SymbolType>
   static BitmapSizePerSymbol calculateBitmapSizePerSymbol(
      const SequenceStore<SymbolType>& seq_store
   );

   template <typename SymbolType>
   static BitmapContainerSize calculateBitmapContainerSizePerGenomeSection(
      const SequenceStore<SymbolType>& seq_store,
      size_t section_length
   );
};

}  // namespace silo
