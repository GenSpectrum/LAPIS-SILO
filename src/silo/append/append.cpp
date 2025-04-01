#include "silo/append/append.h"

#include <fstream>
#include <istream>

#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/common/silo_directory.h"
#include "silo/database.h"

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

}  // namespace

int runAppend(const silo::config::AppendConfig& append_config) {
   silo::SiloDirectory silo_directory{append_config.silo_directory};

   silo::SiloDataSource database_state_directory =
      getMostRecentOrSpecifiedDatabaseState(silo_directory, append_config.silo_data_source);

   std::shared_ptr<Database> database =
      std::make_shared<Database>(Database::loadDatabaseState(database_state_directory));

   auto input = silo::InputStreamWrapper::openFileOrStdIn(append_config.append_file);
   silo::append::appendDataToDatabase(
      *database, silo::append::NdjsonLineReader{input.getInputStream()}
   );

   const silo::DataVersion data_version =
      getDataVersionFromStringOrMineNewDataVersion(append_config.data_version);

   database->setDataVersion(data_version);

   database->saveDatabaseState(append_config.silo_directory);

   SPDLOG_INFO("{}", database->getDatabaseInfo());

   return 0;
}
