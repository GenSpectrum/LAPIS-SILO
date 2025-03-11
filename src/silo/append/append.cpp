#include "silo/append/append.h"

#include <fstream>
#include <istream>

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

std::unique_ptr<std::istream> openInputFileOrStdIn(
   const std::optional<std::filesystem::path>& input_file
) {
   if (input_file.has_value()) {
      // TODO maybe zstd compressed
      auto file = std::make_unique<std::ifstream>(input_file.value());
      if (!file->is_open()) {
         std::cerr << "Error: Unable to open file!" << std::endl;
         throw std::runtime_error("TODO valid error message");  // TODO valid error
      }
      return file;
   } else {
      return std::make_unique<std::istream>(std::cin.rdbuf());  // Wrap std::cin in a unique_ptr
   }
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

}  // namespace

int runAppend(const silo::config::AppendConfig& append_config) {
   silo::SiloDirectory silo_directory{append_config.silo_directory};

   silo::SiloDataSource database_state_directory =
      getMostRecentOrSpecifiedDatabaseState(silo_directory, append_config.silo_data_source);

   std::shared_ptr<Database> database =
      std::make_shared<Database>(Database::loadDatabaseState(database_state_directory));

   silo::DatabaseInserter database_inserter(database);

   // TODO make partition configurable
   silo::DatabasePartitionInserter partition_inserter = database_inserter.openNewPartition();

   std::unique_ptr<std::istream> input = openInputFileOrStdIn(append_config.append_file);

   // TODO refactor following lines to keep business logic separate
   std::string line;
   size_t count = 0;
   while (std::getline(*input, line)) {  // Read file line by line
      if (line.empty())
         continue;  // Skip empty lines

      SPDLOG_INFO("Inserting line {}", count++);

      try {
         nlohmann::json json_obj = nlohmann::json::parse(line);

         partition_inserter.insert(json_obj);

      } catch (const std::exception& e) {
         std::cerr << "Error parsing JSON: " << e.what() << std::endl;
         return 1;
      }
   }

   const silo::DataVersion data_version =
      getDataVersionFromStringOrMineNewDataVersion(append_config.data_version);

   database->setDataVersion(data_version);

   database->saveDatabaseState(append_config.silo_directory);

   SPDLOG_INFO("{}", database->getDatabaseInfo());

   return 0;
}
