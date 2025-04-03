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
#include "silo/schema/database_schema.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/table.h"
#include "silo/storage/table_partition.h"

namespace silo {

class Database {
  public:
   schema::DatabaseSchema schema;
   storage::Table table;

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database(silo::schema::DatabaseSchema database_schema);

   virtual ~Database() = default;

   void validate() const;

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   void setDataVersion(const DataVersion& data_version);
   virtual DataVersion::Timestamp getDataVersionTimestamp() const;

   virtual query_engine::QueryResult executeQuery(const std::string& query) const;

  private:
};

}  // namespace silo
