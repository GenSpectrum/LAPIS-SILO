#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/data_version.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/sequence_store.h"

namespace silo {
struct BitmapContainerSize;
struct BitmapSizePerSymbol;
struct DatabaseInfo;
struct DetailedDatabaseInfo;
struct ReferenceGenomes;
}  // namespace silo
namespace silo::preprocessing {
struct Partitions;
struct PreprocessingConfig;
}  // namespace silo::preprocessing

namespace silo {

class Database {
  public:
   silo::config::DatabaseConfig database_config;
   std::vector<DatabasePartition> partitions;

   silo::storage::ColumnGroup columns;
   std::map<std::string, SequenceStore<Nucleotide>> nuc_sequences;
   std::map<std::string, SequenceStore<AminoAcid>> aa_sequences;

   static Database preprocessing(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config_
   );

   static Database preprocessingNdjson(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config_
   );

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const std::filesystem::path& save_directory);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DetailedDatabaseInfo detailedDatabaseInfo() const;

   [[nodiscard]] const PangoLineageAliasLookup& getAliasKey() const;

   void setDataVersion(const DataVersion& data_version);
   virtual DataVersion getDataVersion() const;

   template <typename SymbolType>
   std::optional<std::string> getDefaultSequenceName() const;

   template <typename SymbolType>
   std::vector<std::string> getSequenceNames() const;

   template <typename SymbolType>
   const std::map<std::string, SequenceStore<SymbolType>>& getSequenceStores() const;

   virtual query_engine::QueryResult executeQuery(const std::string& query) const;

  private:
   PangoLineageAliasLookup alias_key;
   DataVersion data_version_ = DataVersion{""};

   void build(
      duckdb::Connection& connection,
      const preprocessing::Partitions& partition_descriptor,
      const ReferenceGenomes& reference_genomes
   );

   std::map<std::string, std::vector<Nucleotide::Symbol>> getNucSequences() const;

   std::map<std::string, std::vector<AminoAcid::Symbol>> getAASequences() const;

   void initializeColumns();
   void initializeColumn(config::ColumnType column_type, const std::string& name);
   void initializeNucSequences(
      const std::map<std::string, std::vector<Nucleotide::Symbol>>& reference_sequences
   );
   void initializeAASequences(
      const std::map<std::string, std::vector<AminoAcid::Symbol>>& reference_sequences
   );
   void finalizeInsertionIndexes();

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
