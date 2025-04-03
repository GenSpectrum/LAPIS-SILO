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

}  // namespace

namespace silo::append {

int runAppend(const silo::config::AppendConfig& append_config) {
   const silo::SiloDirectory silo_directory{append_config.silo_directory};

   const auto database_state_directory =
      getMostRecentOrSpecifiedDatabaseState(silo_directory, append_config.silo_data_source);

   const auto database =
      std::make_shared<Database>(Database::loadDatabaseState(database_state_directory));

   auto input = silo::InputStreamWrapper::openFileOrStdIn(append_config.append_file);
   silo::append::appendDataToDatabase(
      *database, silo::append::NdjsonLineReader{input.getInputStream()}
   );

   database->saveDatabaseState(append_config.silo_directory);

   SPDLOG_INFO("{}", database->getDatabaseInfo());

   return 0;
}

}  // namespace silo::append
