#pragma once

#include <filesystem>

#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"
#include "silo/database_info.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo {

class Database {
  public:
   schema::DatabaseSchema schema;
   std::shared_ptr<storage::Table> table;

   void updateDataVersion();

  private:
   DataVersion data_version_ = DataVersion::mineDataVersion();

  public:
   explicit Database(silo::schema::DatabaseSchema database_schema);

   virtual ~Database() = default;

   void saveDatabaseState(const std::filesystem::path& save_directory);

   static Database loadDatabaseState(const silo::SiloDataSource& silo_data_source);

   [[nodiscard]] virtual DatabaseInfo getDatabaseInfo() const;

   [[nodiscard]] virtual DataVersion::Timestamp getDataVersionTimestamp() const;
};

}  // namespace silo
