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
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/preprocessing/partition.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/storage/unaligned_sequence_store.h"

namespace silo::preprocessing {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Preprocessor;
}  // namespace silo::preprocessing

namespace silo {

class Database {
   friend class preprocessing::Preprocessor;

  public:
   silo::config::DatabaseConfig database_config;
   std::vector<DatabasePartition> partitions;
   std::filesystem::path unaligned_sequences_directory;

   silo::storage::ColumnGroup columns;

   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

   std::map<std::string, SequenceStore<Nucleotide>> nuc_sequences;
   std::map<std::string, SequenceStore<AminoAcid>> aa_sequences;
   std::map<std::string, UnalignedSequenceStore> unaligned_nuc_sequences;

  private:
   common::LineageTreeAndIdMap lineage_tree;
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database(silo::config::DatabaseConfig&& database_config)
       : database_config(database_config) {}

   Database() = delete;

   Database(const Database&) = default;
   Database(Database&&) = default;
   Database& operator=(const Database&) = default;
   Database& operator=(Database&&) = default;

   virtual ~Database() = default;

   void validate() const;

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const std::filesystem::path& save_directory);

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
   void initializeNucSequences(
      const std::vector<std::string>& sequence_names,
      const std::vector<std::vector<Nucleotide::Symbol>>& reference_sequences
   );
   void initializeAASequences(
      const std::vector<std::string>& sequence_names,
      const std::vector<std::vector<AminoAcid::Symbol>>& reference_sequences
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
