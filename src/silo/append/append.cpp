#include "silo/append/append.h"

#include <fstream>
#include <istream>

#include "silo/common/silo_directory.h"
#include "silo/database.h"

using silo::Database;

class AppendError : public std::runtime_error {
  public:
   AppendError(std::string error_message) : std::runtime_error(error_message) {}
};

namespace silo{

class DatabasePartitionInserter{
   std::shared_ptr<DatabasePartition> database_partition;

  public:
   DatabasePartitionInserter(std::shared_ptr<DatabasePartition> database_partition) : database_partition(database_partition)  {}

   ~DatabasePartitionInserter() {
      database_partition->validate();

      for (auto& [_, sequence_store] : database_partition->nuc_sequences) {
         sequence_store.finalize();
      }
      for (auto& [_, sequence_store] : database_partition->aa_sequences) {
         sequence_store.finalize();
      }
   }

   void insert(const nlohmann::json& ndjson_line){
      for (auto& column_metadata : database_partition->columns.metadata) {
         const auto& value = ndjson_line["metadata"][column_metadata.name];
         database_partition->columns.addJsonValueToColumn(column_metadata.name, column_metadata.type, value);
      }
      SPDLOG_INFO(
         "date count is {}", database_partition->columns.date_columns.begin()->second.getValues().size()
      );

      for (const auto& [sequence_name, sequence_store] : database_partition->nuc_sequences) {
         auto& sequence_read = sequence_store.appendNewSequenceRead();

         const auto& sequence_in_json = ndjson_line["alignedNucleotideSequences"][sequence_name];
         if (sequence_in_json.is_string()) {
            sequence_read.is_valid = true;
            sequence_read.offset = 0;
            sequence_read.sequence = sequence_in_json;
         } else {
            sequence_read.is_valid = false;
         }
      }
      for (const auto& [sequence_name, sequence_store] : database_partition->aa_sequences) {
         auto& sequence_read = sequence_store.appendNewSequenceRead();

         const auto& sequence_in_json = ndjson_line["alignedAminoAcidSequences"][sequence_name];
         if (sequence_in_json.is_string()) {
            sequence_read.is_valid = true;
            sequence_read.offset = 0;
            sequence_read.sequence = sequence_in_json;
         } else {
            sequence_read.is_valid = false;
         }
      }

      database_partition->sequence_count++;
   }
};

class DatabaseInserter{
   std::shared_ptr<silo::Database> database;

  public:
   DatabaseInserter(std::shared_ptr<silo::Database> database) : database(database) {}

   ~DatabaseInserter() {
      database->validate();
   }

   DatabasePartitionInserter openNewPartition(){
      return DatabasePartitionInserter{ database->addPartition() };
   }
};
}

namespace {

silo::SiloDataSource getMostRecentOrSpecifiedDatabaseState(const silo::SiloDirectory& silo_directory, const std::optional<std::filesystem::path>& specified_directory){
   if(specified_directory.has_value()){
      auto specified_data_source = silo::SiloDataSource::checkValidDataSource(specified_directory.value());
      if(specified_data_source == std::nullopt){
         throw AppendError{"The specified siloDataSource directory is not valid data-source."};
      }
   }
   SPDLOG_INFO("No data directory specified, automatically using the most recent one in the silo-directory {}", silo_directory);
   auto most_recent_data_directory = silo_directory.getMostRecentDataDirectory();
   if(most_recent_data_directory == std::nullopt){
      throw AppendError{"No data directory specified and the silo-directory does not contain any valid data source."};
   }
   return most_recent_data_directory.value();
}

std::unique_ptr<std::istream> openInputFileOrStdIn(const std::optional<std::filesystem::path>& input_file) {
   if (input_file.has_value()) {
      // TODO maybe zstd compressed
      auto file = std::make_unique<std::ifstream>(input_file.value());
      if (!file->is_open()) {
         std::cerr << "Error: Unable to open file!" << std::endl;
         throw std::runtime_error("TODO valid error message"); // TODO valid error
      }
      return file;
   } else {
      return std::make_unique<std::istream>(std::cin.rdbuf());  // Wrap std::cin in a unique_ptr
   }
}

}

int runAppend(const silo::config::AppendConfig& append_config) {
   silo::SiloDirectory silo_directory{append_config.silo_directory};

   silo::SiloDataSource database_state_directory = getMostRecentOrSpecifiedDatabaseState(silo_directory, append_config.silo_data_source);

   std::shared_ptr<Database> database = std::make_shared<Database>(Database::loadDatabaseState(database_state_directory));

   silo::DatabaseInserter database_inserter(database);

   silo::DatabasePartitionInserter partition_inserter = database_inserter.openNewPartition();

   std::unique_ptr<std::istream> input = openInputFileOrStdIn(append_config.append_file);


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

   database->setDataVersion(silo::DataVersion::mineDataVersion());

   database->saveDatabaseState(append_config.silo_directory);

   SPDLOG_INFO("{}", database->getDatabaseInfo());

   return 0;
}
