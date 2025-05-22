#include "silo/database.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>

#include "silo/common/data_version.h"
#include "silo/common/file_to_string.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/common/silo_directory.h"
#include "silo/common/version.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/serialize_optional.h"
#include "silo/storage/table_partition.h"

namespace silo {

Database::Database(schema::DatabaseSchema database_schema)
    : schema(database_schema),
      table(std::make_shared<storage::Table>(schema.getDefaultTableSchema())) {}

DatabaseInfo Database::getDatabaseInfo() const {
   std::atomic<uint32_t> sequence_count = 0;
   std::atomic<uint64_t> total_size = 0;
   std::atomic<size_t> nucleotide_symbol_n_bitmaps_size = 0;

   for (size_t partition_idx = 0; partition_idx < table->getNumberOfPartitions(); ++partition_idx) {
      const storage::TablePartition& table_partition = table->getPartition(partition_idx);
      // TODO(#743) also add other size estimates, and try to analyze its accuracy to RSS in general
      for (const auto& [_, seq_column] : table_partition.columns.nuc_columns) {
         total_size += seq_column.computeSize();
         for (const auto& bitmap : seq_column.missing_symbol_bitmaps) {
            nucleotide_symbol_n_bitmaps_size += bitmap.getSizeInBytes(false);
         }
      }
      sequence_count += table_partition.sequence_count;
   }

   return DatabaseInfo{
      .version = silo::RELEASE_VERSION,
      .sequence_count = sequence_count,
      .total_size = total_size,
      .n_bitmaps_size = nucleotide_symbol_n_bitmaps_size,
      .number_of_partitions = table->getNumberOfPartitions()
   };
}

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

   auto yaml_string =
      YAML::Dump(schema::DatabaseSchema{{{schema::TableName::getDefault(), table->schema}}}.toYAML()
      );
   const auto database_schema_filename = versioned_save_directory / "database_schema.yaml";
   std::ofstream database_schema_file{database_schema_filename};
   database_schema_file << yaml_string;
   database_schema_file.close();

   std::string table_name = schema.tables.begin()->first.getName();
   SPDLOG_DEBUG("Saving table data");
   std::filesystem::create_directory(versioned_save_directory / table_name);
   table->saveData(versioned_save_directory / table_name);

   data_version_.saveToFile(versioned_save_directory / "data_version.silo");
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

   const auto database_schema_filename = save_directory / "database_schema.yaml";
   auto yaml_string = common::fileToString(database_schema_filename);
   if (yaml_string == std::nullopt) {
      throw silo::persistence::LoadDatabaseException(
         fmt::format("Could not load DatabaseSchema from {}", database_schema_filename)
      );
   }
   YAML::Node yaml_schema = YAML::Load(yaml_string.value());
   schema::DatabaseSchema schema = schema::DatabaseSchema::fromYAML(yaml_schema);

   Database database{schema};

   std::string table_name = schema.tables.begin()->first.getName();
   SPDLOG_DEBUG("Loading data for table ");
   database.table->loadData(save_directory / table_name);

   database.data_version_ = loadDataVersion(save_directory / "data_version.silo");

   SPDLOG_INFO(
      "Finished loading data_version from {}", (save_directory / "data_version.silo").string()
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
