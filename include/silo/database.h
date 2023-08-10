#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "silo/common/data_version.h"
#include "silo/config/database_config.h"
#include "silo/storage/aa_store.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"

namespace silo {
struct BitmapContainerSize;
struct BitmapSizePerSymbol;
struct DatabaseInfo;
struct DetailedDatabaseInfo;
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
   std::map<std::string, SequenceStore> nuc_sequences;
   std::map<std::string, AAStore> aa_sequences;

   static Database preprocessing(
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

  private:
   PangoLineageAliasLookup alias_key;
   DataVersion data_version_ = {""};

   void build(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const preprocessing::Partitions& partition_descriptor,
      const ReferenceGenomes& reference_genomes
   );

   std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>> getNucSequences() const;

   std::map<std::string, std::vector<AA_SYMBOL>> getAASequences() const;

   void initializeColumns();
   void initializeColumn(config::ColumnType column_type, const std::string& name);
   void initializeNucSequences(
      const std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>>& reference_sequences
   );
   void initializeAASequences(
      const std::map<std::string, std::vector<AA_SYMBOL>>& reference_sequences
   );
   void finalizeInsertionIndexes();

   static BitmapSizePerSymbol calculateBitmapSizePerSymbol(const SequenceStore& seq_store);

   static BitmapContainerSize calculateBitmapContainerSizePerGenomeSection(
      const SequenceStore& seq_store,
      size_t section_length
   );
};

}  // namespace silo

#endif  // SILO_DATABASE_H
