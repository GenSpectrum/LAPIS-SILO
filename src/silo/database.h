#pragma once

#include <filesystem>
#include <vector>

#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"
#include "silo/database_info.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo {

class Database {
  public:
   schema::DatabaseSchema schema;
   std::map<schema::TableName, std::shared_ptr<storage::Table>> tables;

   void updateDataVersion();

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   Database() = default;

   explicit Database(silo::schema::DatabaseSchema database_schema);

   virtual ~Database() = default;

   void createTable(std::string table_name, silo::schema::TableSchema table_schema);

   void createSimpleTable(const std::string& table_name, const std::vector<schema::ColumnIdentifier>& columns) {}

   void appendData(std::string table_name, std::string file_name);

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DataVersion::Timestamp getDataVersionTimestamp() const;
};

}  // namespace silo
