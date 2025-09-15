#include "silo/database.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "silo/common/data_version.h"
#include "silo/common/silo_directory.h"
#include "silo/common/version.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table_partition.h"

namespace silo {

Database::Database(schema::DatabaseSchema database_schema)
    : schema(database_schema),
      table(std::make_shared<storage::Table>(schema.getDefaultTableSchema())) {}

DatabaseInfo Database::getDatabaseInfo() const {
   uint32_t sequence_count = 0;
   uint64_t vertical_bitmaps_size = 0;
   size_t horizontal_bitmaps_size = 0;

   for (size_t partition_idx = 0; partition_idx < table->getNumberOfPartitions(); ++partition_idx) {
      const storage::TablePartition& table_partition = table->getPartition(partition_idx);
      // TODO(#743) try to analyze size accuracy relative to RSS
      for (const auto& [_, seq_column] : table_partition.columns.nuc_columns) {
         auto info = seq_column.getInfo();
         vertical_bitmaps_size += info.vertical_bitmaps_size;
         horizontal_bitmaps_size += info.horizontal_bitmaps_size;
      }
      for (const auto& [_, seq_column] : table_partition.columns.aa_columns) {
         auto info = seq_column.getInfo();
         vertical_bitmaps_size += info.vertical_bitmaps_size;
         horizontal_bitmaps_size += info.horizontal_bitmaps_size;
      }
      sequence_count += table_partition.sequence_count;
   }

   return DatabaseInfo{
      .version = silo::RELEASE_VERSION,
      .sequence_count = sequence_count,
      .vertical_bitmaps_size = vertical_bitmaps_size,
      .horizontal_bitmaps_size = horizontal_bitmaps_size,
      .number_of_partitions = table->getNumberOfPartitions()
   };
}

const std::string DATABASE_SCHEMA_FILENAME = "database_schema.silo";
const std::string DATA_VERSION_FILENAME = "data_version.silo";

void Database::saveDatabaseState(const std::filesystem::path& save_directory) {
   if (getDataVersionTimestamp().value.empty()) {
      throw persistence::SaveDatabaseException(
         "Corrupted database (Data version is empty). Cannot save database."
      );
   }

   std::filesystem::create_directory(save_directory);
   if (!std::filesystem::exists(save_directory)) {
      auto error = fmt::format(
         "Could not create the directory '{}' which contains the saved databases outputs",
         save_directory.string()
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   const std::filesystem::path versioned_save_directory =
      save_directory / getDataVersionTimestamp().value;
   SPDLOG_INFO("Saving database to '{}'", versioned_save_directory.string());

   if (std::filesystem::exists(versioned_save_directory)) {
      auto error = fmt::format(
         "In the output directory {} there already exists a file/folder with the name equal to "
         "the current data-version: {}",
         save_directory.string(),
         getDataVersionTimestamp().value
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   std::filesystem::create_directory(versioned_save_directory);

   SPDLOG_INFO("Saving database schema");

   const auto database_schema_path = versioned_save_directory / DATABASE_SCHEMA_FILENAME;
   schema.saveToFile(database_schema_path);

   std::string table_name = schema.tables.begin()->first.getName();
   SPDLOG_DEBUG("Saving table data");
   std::filesystem::create_directory(versioned_save_directory / table_name);
   table->saveData(versioned_save_directory / table_name);

   data_version_.saveToFile(versioned_save_directory / DATA_VERSION_FILENAME);
}

namespace {
DataVersion loadDataVersion(const std::filesystem::path& filename) {
   if (!std::filesystem::is_regular_file(filename)) {
      auto error = fmt::format("Input file {} could not be opened.", filename.string());
      throw persistence::LoadDatabaseException(error);
   }
   auto data_version = DataVersion::fromFile(filename);
   if (data_version == std::nullopt) {
      auto error_message = fmt::format(
         "Data version file {} did not contain a valid data version", filename.string()
      );
      SPDLOG_ERROR(error_message);
      throw persistence::LoadDatabaseException(error_message);
   }
   return data_version.value();
}
}  // namespace

Database Database::loadDatabaseState(const silo::SiloDataSource& silo_data_source) {
   SPDLOG_INFO("Loading database from data source: {}", silo_data_source.toDebugString());
   const auto save_directory = silo_data_source.path;

   const auto database_schema_path = save_directory / DATABASE_SCHEMA_FILENAME;
   auto schema = schema::DatabaseSchema::loadFromFile(database_schema_path);

   Database database{schema};

   std::string table_name = schema.tables.begin()->first.getName();
   SPDLOG_DEBUG("Loading data for table ");
   database.table->loadData(save_directory / table_name);

   database.data_version_ = loadDataVersion(save_directory / DATA_VERSION_FILENAME);

   SPDLOG_INFO(
      "Finished loading data_version from {}", (save_directory / DATA_VERSION_FILENAME).string()
   );
   SPDLOG_INFO("Database info after loading: {}", database.getDatabaseInfo());

   return database;
}

DataVersion::Timestamp Database::getDataVersionTimestamp() const {
   return data_version_.timestamp;
}

void Database::updateDataVersion() {
   data_version_ = DataVersion::mineDataVersion();
   SPDLOG_DEBUG("Data version was set to {}", data_version_.toString());
}

}  // namespace silo
