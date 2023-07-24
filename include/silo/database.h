#ifndef SILO_DATABASE_H
#define SILO_DATABASE_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "silo/config/database_config.h"
#include "silo/storage/aa_store.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"

namespace silo {
namespace preprocessing {
struct Partitions;
struct PreprocessingConfig;
}  // namespace preprocessing
struct BitmapContainerSize;
struct BitmapSizePerSymbol;
struct DatabaseInfo;
struct DetailedDatabaseInfo;
}  // namespace silo

namespace silo {
class Database {
  public:
   silo::config::DatabaseConfig database_config;
   ReferenceGenomes reference_genomes;
   std::vector<DatabasePartition> partitions;

   std::unordered_map<std::string, storage::column::StringColumn> string_columns;
   std::unordered_map<std::string, storage::column::IndexedStringColumn> indexed_string_columns;
   std::unordered_map<std::string, storage::column::IntColumn> int_columns;
   std::unordered_map<std::string, storage::column::FloatColumn> float_columns;
   std::unordered_map<std::string, storage::column::DateColumn> date_columns;
   std::unordered_map<std::string, storage::column::PangoLineageColumn> pango_lineage_columns;
   std::unordered_map<std::string, storage::column::InsertionColumn> insertion_columns;

   std::unordered_map<std::string, SequenceStore> nuc_sequences;
   std::unordered_map<std::string, AAStore> aa_sequences;

   Database();

   void preprocessing(
      const preprocessing::PreprocessingConfig& preprocessing_config,
      const config::DatabaseConfig& database_config_
   );

   void build(
      const std::filesystem::path& input_folder,
      const preprocessing::Partitions& partition_descriptor
   );

   virtual silo::DatabaseInfo getDatabaseInfo() const;

   virtual DetailedDatabaseInfo detailedDatabaseInfo() const;

   [[maybe_unused]] void flipBitmaps();

   [[maybe_unused]] void saveDatabaseState(
      const std::string& save_directory,
      const silo::preprocessing::Partitions& partition_descriptor
   );

   [[maybe_unused]] [[maybe_unused]] void loadDatabaseState(const std::string& save_directory);

   [[nodiscard]] const PangoLineageAliasLookup& getAliasKey() const;

  private:
   PangoLineageAliasLookup alias_key;

   void initializeColumns();
   void initializeColumn(config::ColumnType column_type, const std::string& name);
   void initializeSequences();
   void finalizeInsertionIndexes();

   static BitmapSizePerSymbol calculateBitmapSizePerSymbol(const SequenceStore& seq_store);

   static BitmapContainerSize calculateBitmapContainerSizePerGenomeSection(
      const SequenceStore& seq_store,
      size_t section_length
   );
};

std::string buildChunkString(uint32_t partition, uint32_t chunk);

}  // namespace silo

#endif  // SILO_DATABASE_H
