#pragma once

#include <memory>

#include "silo/database.h"
#include "silo/storage/database_partition.h"

namespace silo {

class DatabasePartitionInserter {
   std::shared_ptr<DatabasePartition> database_partition;

  public:
   DatabasePartitionInserter(std::shared_ptr<DatabasePartition> database_partition)
       : database_partition(database_partition) {}

   ~DatabasePartitionInserter() {
      database_partition->validate();

      for (auto& [_, sequence_store] : database_partition->nuc_sequences) {
         sequence_store.finalize();
      }
      for (auto& [_, sequence_store] : database_partition->aa_sequences) {
         sequence_store.finalize();
      }
   }

   void insert(const nlohmann::json& ndjson_line) {
      for (auto& column_metadata : database_partition->columns.metadata) {
         const auto& value = ndjson_line["metadata"][column_metadata.name];
         database_partition->columns.addJsonValueToColumn(
            column_metadata.name, column_metadata.type, value
         );
      }
      SPDLOG_INFO(
         "date count is {}",
         database_partition->columns.date_columns.begin()->second.getValues().size()
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

class DatabaseInserter {
   std::shared_ptr<silo::Database> database;

  public:
   DatabaseInserter(std::shared_ptr<silo::Database> database)
       : database(database) {}

   ~DatabaseInserter() { database->validate(); }

   DatabasePartitionInserter openNewPartition() {
      return DatabasePartitionInserter{database->addPartition()};
   }
};

}  // namespace silo