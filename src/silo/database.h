#pragma once

#include <filesystem>

#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"
#include "silo/database_info.h"
#include "silo/query_engine/query.h"
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

   void createTable(schema::TableName table_name, silo::schema::TableSchema table_schema);

   void appendData(schema::TableName table_name, std::istream& input_stream);

   void query(query_engine::Query query);

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DataVersion::Timestamp getDataVersionTimestamp() const;

   [[nodiscard]] query_engine::QueryPlan createQueryPlan(
      const query_engine::Query& query,
      const config::QueryOptions& query_options,
      std::string_view request_id
   ) const;
};

}  // namespace silo
