#include "silo/append/append.h"

#include <fstream>
#include <istream>

#include "silo/common/input_stream_wrapper.h"
#include "silo/common/silo_directory.h"
#include "silo/database.h"
#include "silo/database_inserter.h"

using silo::Database;

class AppendError : public std::runtime_error {
  public:
   AppendError(std::string error_message)
       : std::runtime_error(error_message) {}
};

namespace {

silo::SiloDataSource getMostRecentOrSpecifiedDatabaseState(
   const silo::SiloDirectory& silo_directory,
   const std::optional<std::filesystem::path>& specified_directory
) {
   if (specified_directory.has_value()) {
      auto specified_data_source =
         silo::SiloDataSource::checkValidDataSource(specified_directory.value());
      if (specified_data_source == std::nullopt) {
         throw AppendError{"The specified siloDataSource directory is not valid data-source."};
      }
   }
   SPDLOG_INFO(
      "No data directory specified, automatically using the most recent one in the silo-directory "
      "{}",
      silo_directory
   );
   auto most_recent_data_directory = silo_directory.getMostRecentDataDirectory();
   if (most_recent_data_directory == std::nullopt) {
      throw AppendError{
         "No data directory specified and the silo-directory does not contain any valid data "
         "source."
      };
   }
   return most_recent_data_directory.value();
}

silo::DataVersion getDataVersionFromStringOrMineNewDataVersion(
   std::optional<std::string> data_version
) {
   if (data_version.has_value()) {
      auto maybe_timestamp = silo::DataVersion::Timestamp::fromString(data_version.value());
      if (!maybe_timestamp.has_value()) {
         throw AppendError("The specified dataVersion: {} is not a valid Unix timestamp");
      }
      return silo::DataVersion::mineDataVersionFromTimestamp(maybe_timestamp.value());
   }
   return silo::DataVersion::mineDataVersion();
}

void appendDataToTable(
   silo::storage::Table& table,
   const silo::config::AppendConfig& append_config
) {
   silo::TableInserter table_inserter(&table);

   // TODO make partition configurable
   silo::TablePartitionInserter partition_inserter = table_inserter.openNewPartition();

   auto input = silo::InputStreamWrapper::openFileOrStdIn(append_config.append_file);

   // TODO refactor following lines to keep business logic separate
   std::string line;
   size_t line_count = 0;
   while (std::getline(input.getInputStream(), line)) {  // Read file line by line
      if (line.empty())
         continue;  // Skip empty lines

      SPDLOG_DEBUG("Inserting line {}", line_count);

      try {
         nlohmann::json json_obj = nlohmann::json::parse(line);

         partition_inserter.insert(json_obj);

      } catch (const nlohmann::json::parse_error& e) {
         std::cerr << "Error parsing JSON: " << e.what() << std::endl;
         // TODO throw silo::append::AppendException()
         SILO_PANIC("Error parsing JSON: {}", e.what());
      }
      line_count++;
      if (line_count % 10000 == 0) {
         SPDLOG_INFO("Processed {} lines from the input file", line_count);
      }
   }
}

}  // namespace

int runAppend(const silo::config::AppendConfig& append_config) {
   silo::SiloDirectory silo_directory{append_config.silo_directory};

   silo::SiloDataSource database_state_directory =
      getMostRecentOrSpecifiedDatabaseState(silo_directory, append_config.silo_data_source);

   std::shared_ptr<Database> database =
      std::make_shared<Database>(Database::loadDatabaseState(database_state_directory));

   appendDataToTable(database->table, append_config);

   const silo::DataVersion data_version =
      getDataVersionFromStringOrMineNewDataVersion(append_config.data_version);

   database->setDataVersion(data_version);

   database->saveDatabaseState(append_config.silo_directory);

   SPDLOG_INFO("{}", database->getDatabaseInfo());

   return 0;
}
