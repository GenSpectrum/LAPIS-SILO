#include "silo/append/append.h"

#include "silo/database.h"

using silo::Database;

int runAppend(const silo::config::AppendConfig& append_config) {
   Database database = Database::loadDatabaseState(append_config.database_state);

   auto& table = database.partitions.at(0);

   std::ifstream file(append_config.append_file);  // Open the NDJSON file
   if (!file.is_open()) {
      std::cerr << "Error: Unable to open file!" << std::endl;
      return 1;
   }

   std::string line;
   while (std::getline(file, line)) {  // Read file line by line
      if (line.empty())
         continue;  // Skip empty lines

      try {
         nlohmann::json json_obj = nlohmann::json::parse(line);

         for (auto& column_metadata : database.columns.metadata) {
            const auto& value = json_obj["metadata"][column_metadata.name];
            table.columns.addJsonValueToColumn(column_metadata.name, column_metadata.type, value);
         }

         for (auto& sequence_name : database.nuc_sequence_names) {
            auto& sequence_read = table.nuc_sequences.at(sequence_name).appendNewSequenceRead();

            const auto& sequence_in_json = json_obj["alignedNucleotideSequences"][sequence_name];
            if (sequence_in_json.is_string()) {
               sequence_read.is_valid = true;
               sequence_read.offset = 0;
               sequence_read.sequence = sequence_in_json;
            } else {
               sequence_read.is_valid = false;
            }
         }
         for (auto& sequence_name : database.aa_sequence_names) {
            auto& sequence_read = table.aa_sequences.at(sequence_name).appendNewSequenceRead();

            const auto& sequence_in_json = json_obj["alignedAminoAcidSequences"][sequence_name];
            if (sequence_in_json.is_string()) {
               sequence_read.is_valid = true;
               sequence_read.offset = 0;
               sequence_read.sequence = sequence_in_json;
            } else {
               sequence_read.is_valid = false;
            }
         }
      } catch (const std::exception& e) {
         std::cerr << "Error parsing JSON: " << e.what() << std::endl;
         return 1;
      }
   }
   file.close();

   for (auto& sequence_name : database.nuc_sequence_names) {
      table.nuc_sequences.at(sequence_name).finalize();
   }
   for (auto& sequence_name : database.aa_sequence_names) {
      table.aa_sequences.at(sequence_name).finalize();
   }

   database.setDataVersion(silo::DataVersion::mineDataVersion());

   database.saveDatabaseState(append_config.output_folder);

   return 0;
}
